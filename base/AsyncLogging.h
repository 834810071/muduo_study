//
// Created by jxq on 19-11-16.
//

#ifndef MYMUDUO_ASYNCLOGGING_H
#define MYMUDUO_ASYNCLOGGING_H

#include "BlockingQueue.h"
#include "BoundedBlockingQueue.h"
#include "CountDownLatch.h"
#include "Mutex.h"
#include "Thread.h"
#include "LogStream.h"

#include <atomic>
#include <vector>

namespace muduo
{

    class AsyncLogging : noncopyable
    {
    public:

        AsyncLogging(const string& basename,
                     off_t rollSize,
                     int flushInterval = 3);

        ~AsyncLogging()
        {
            if (running_)
            {
                stop();
            }
        }

        void append(const char* logline, int len);

        void start()
        {
            // 在构造函数中latch_的值为1
            // 线程运行之后将latch_的减为0
            running_ = true;
            thread_.start();
            // 必须等到latch_变为0才能从start函数中返回，这表明初始化已经完成
            latch_.wait();
        }

        void stop() NO_THREAD_SAFETY_ANALYSIS
        {
            running_ = false;
            cond_.notify();
            thread_.join();
        }

    private:

        void threadFunc();

        typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer;
        // 用unique_ptr管理buffer，持有对对象的独有权，不能进行复制操作只能进行移动操作（效率更高）
        typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
        typedef BufferVector::value_type BufferPtr; // 指向buffer的指针

        const int flushInterval_; // 定期（flushInterval_秒）将缓冲区的数据写到文件中
        std::atomic<bool> running_; // 是否正在运行
        const string basename_; // 日志名字
        const off_t rollSize_; // 预留的日志大小
        muduo::Thread thread_; // 执行该异步日志记录器的线程
        muduo::CountDownLatch latch_; // 倒计时计数器初始化为1，用于指示什么时候日志记录器才能开始正常工作
        muduo::MutexLock mutex_;
        muduo::Condition cond_ GUARDED_BY(mutex_);
        BufferPtr currentBuffer_ GUARDED_BY(mutex_); // 当前的缓冲区
        BufferPtr nextBuffer_ GUARDED_BY(mutex_); // 下一个缓冲区
        BufferVector buffers_ GUARDED_BY(mutex_); // 缓冲区队列
    };

}  // namespace muduo

#endif //MYMUDUO_ASYNCLOGGING_H
