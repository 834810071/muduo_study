//
// Created by jxq on 19-11-16.
//

#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"

#include <stdio.h>

using namespace muduo;

AsyncLogging::AsyncLogging(const string &basename,
                           off_t rollSize,
                           int flushInterval)
    : flushInterval_(flushInterval),
      running_(false),
      basename_(basename),
      rollSize_(rollSize),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),   // thread绑定threadFunc回调函数
      latch_(1),
      mutex_(),
      cond_(mutex_),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer),
      buffers_()
{
    currentBuffer_->bzero();    // 缓冲区清零
    nextBuffer_->bzero();
    buffers_.reserve(16);   // vector预定大小，避免自动增长（效率更高）
}

/********************************************************************
Description :
前端在生成一条日志消息时，会调用AsyncLogging::append()。
如果currentBuffer_够用，就把日志内容写入到currentBuffer_中，
如果不够用(就认为其满了)，就把currentBuffer_放到已满buffer数组中，
等待消费者线程（即后台线程）来取。则将预备好的另一块缓冲
（nextBuffer_）移用为当前缓冲区（currentBuffer_）。
*********************************************************************/
void AsyncLogging::append(const char *logline, int len)
{
    muduo::MutexLockGuard lock(mutex_);
    // 如果当前buffer的长度大于要添加的日志记录的长度，即当前buffer还有空间，就添加到当前日志。
    if (currentBuffer_->avail() > len)
    {
        currentBuffer_->append(logline, len);
    }
    // 当前buffer已满。
    else
    {
        // 把当前buffer添加到buffer数组中。
        buffers_.push_back(std::move(currentBuffer_));
        // 如果另一块缓冲区不为空，则将预备好的另一块缓冲区移用为当前缓冲区。
        if (nextBuffer_)    // 存在
        {
            currentBuffer_ = std::move(nextBuffer_);
        }
        // 如果前端写入速度太快了，一下子把两块缓冲都用完了，那么只好分配一块新的buffer，作当前缓冲区。
        else
        {
            currentBuffer_.reset(new Buffer);   // Rarely happens
        }
        // 添加日志记录。
        currentBuffer_->append(logline, len);
        // 通知后端开始写入日志数据。
        cond_.notify();
    }
}

/********************************************************************
Description :
如果buffers_为空，使用条件变量等待条件满足（即前端线程把一个已经满了
的buffer放到了buffers_中或者超时）。将当前缓冲区放到buffers_数组中。
更新当前缓冲区（currentBuffer_）和另一个缓冲区（nextBuffer_）。
将bufferToWrite和buffers_进行swap。这就完成了将写了日志记录的buffer
从前端线程到后端线程的转变。
*********************************************************************/
void AsyncLogging::threadFunc()
{
    assert(running_ == true);
    latch_.countDown();
    LogFile output(basename_, rollSize_, false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();
    BufferVector buffersToWrite; // 写入日志记录文件的BufferVector。
    buffersToWrite.reserve(16);
    while (running_)
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());

        {
            muduo::MutexLockGuard lock(mutex_);
            // 如果buffers_为空，那么表示没有数据需要写入文件，那么就等待指定的时间。
            if (buffers_.empty())   // unusual usage!
            {
                // 超时 或 前端写满一个或者多个buffer
                cond_.waitForSeconds(flushInterval_);
            }
            // 无论cond是因何（一是超时，二是当前缓冲区写满了）而醒来，都要将currentBuffer_放到buffers_中。
            // 如果是因为时间到（3秒）而醒，那么currentBuffer_还没满，此时也要将之写入LogFile中。
            // 如果已经有一个前端buffer满了，那么在前端线程中就已经把一个前端buffer放到buffers_中
            // 了。此时，还是需要把currentBuffer_放到buffers_中（注意，前后放置是不同的buffer，
            // 因为在前端线程中，currentBuffer_已经被换成nextBuffer_指向的buffer了）。
            buffers_.push_back(std::move(currentBuffer_));  // 存储
            // 将新的buffer（newBuffer1）移用为当前缓冲区（currentBuffer_）
            currentBuffer_ = std::move(newBuffer1); // 清空
            // buffers_和buffersToWrite交换数据，此时buffers_所有的数据存放在buffersToWrite，而buffers_变为空。
            buffersToWrite.swap(buffers_);  // 交换存储
            // 如果nextBuffer_为空，将新的buffer（newBuffer2）移用为另一个缓冲区（nextBuffer_）。
            if (!nextBuffer_)   // 不存在
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        // 如果将要写入文件的buffer列表中buffer的个数大于25，那么将多余数据删除。
        // 前端陷入死循环，拼命发送日志消息，超过后端的处理能力，这是典型的生产速度超过消费速度，
        // 会造成数据在内存中的堆积，严重时引发性能问题(可用内存不足)或程序崩溃(分配内存失败)。
        if (buffersToWrite.size() > 25) // ???
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log message at %s, %zd larger buffers\n",
                    Timestamp::now().toFormattedString().c_str(),
                    buffersToWrite.size()-2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            // 丢掉多余日志，以腾出内存，仅保留两块缓冲区
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
        }

        // 写入文件
        for (const auto& buffer : buffersToWrite)
        {
            // FIXME: use unbuffered stdio FILE ? or use ::writev ?
            output.append(buffer->data(), buffer->length());
        }

        // 重新调整buffersToWrite的大小
        // 将buffersToWrite内的buffer重新填充newBuffer1和newBuffer2
        if (buffersToWrite.size() > 2)
        {
            // drop non-bzero-ed buffers, avoid trashing
            buffersToWrite.resize(2);
        }

        // 从buffersToWrite中弹出一个作为newBuffer1
        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        // 从buffersToWrite中弹出一个作为newBuffer2
        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }
        // 清空buffersToWrite
        buffersToWrite.clear();
        output.flush();
    }

    output.flush();
}