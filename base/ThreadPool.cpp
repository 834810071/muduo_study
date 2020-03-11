//
// Created by jxq on 19-7-1.
//

#include "ThreadPool.h"

#include "Exception.h"

#include <assert.h>
#include <stdio.h>

using namespace muduo;
using namespace std;

ThreadPool::ThreadPool(const string &nameArg)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      name_(nameArg),
      maxQueueSize_(0),
      running_(false)
{

}

ThreadPool::~ThreadPool()
{
    if (running_)
    {
        stop();
    }
}

void ThreadPool::start(int numThreads)
{
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i)
    {
        char id[32];
        snprintf(id, sizeof id, "%d", i+1);
        threads_.emplace_back(new muduo::Thread(
                bind(&ThreadPool::runInThread, this), name_+id));
        threads_[i] -> start();
    }

    if (numThreads == 0 && threadInitCallback_)
    {
        threadInitCallback_();
    }
}

void ThreadPool::stop()
{
    {
        // 一定要加锁!!，因为要修改running_，take()里面wait等待的条件里有running，如果不加锁保护running，
        // take()中先执行while判断running为true，接着下面修改了running，也notifyAll了，这时take中才执行wait，那就永远锁住。
        MutexLockGuard lock(mutex_);
        running_ = false;
        notEmpty_.notifyAll();
    }

    for (auto& thr : threads_)
    {
        thr -> join();
    }
}

size_t ThreadPool::queueSize() const
{
    MutexLockGuard lock(mutex_);
    return queue_.size();
}

void ThreadPool::run(Task task)
{
    if (threads_.empty())   // 线程为空
    {
        task();
    }
    else    // 否则，添加到队列中
    {
        MutexLockGuard lock(mutex_);
        while (isFull())
        {
            notFull_.wait();
        }
        assert(!isFull());

        queue_.push_back(move(task));
        notEmpty_.notify();
    }

}

ThreadPool::Task ThreadPool::take()
{
    MutexLockGuard lock(mutex_);

    // 如果任务列表为空且运行
    // 要判断running,如果stop了线程池，那就不要再wait了
    while (queue_.empty() && running_)
    {
        // 任务列表为空，等待
        notEmpty_.wait();
    }

    Task task;
    if (!queue_.empty())
    {
        task = queue_.front();
        queue_.pop_front();
        if (maxQueueSize_ > 0)
        {
            // 任务列表有空位
            notFull_.notify();
        }
    }
    return task;
}

bool ThreadPool::isFull() const
{
    mutex_.assertLocked();
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

// 线程执行函数 ：循环 调用 take 获取任务，并执行任务
void ThreadPool::runInThread()
{
    try
    {
        if (threadInitCallback_)
        {
            threadInitCallback_();
        }
        while (running_)
        {
            Task task(take());
            if (task)   // 要判断任务是否为空，因为stop的时候会返回空task
            {
                task();
            }
        }
    }
    catch (const Exception& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
        abort();
    }
    catch (const std::exception& ex)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw; // rethrow
    }
}



























