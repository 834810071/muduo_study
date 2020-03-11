//
// Created by jxq on 19-7-1.
//

#ifndef MYMUDUO_THREADPOOL_H
#define MYMUDUO_THREADPOOL_H

#include "Condition.h"
#include "Mutex.h"
#include "Thread.h"
#include "Types.h"

#include <deque>
#include <vector>

using namespace std;

namespace muduo
{
    class ThreadPool : noncopyable{
    public:
        typedef function<void ()> Task;

        explicit ThreadPool(const string& nameArg = string("ThreadPool"));
        ~ThreadPool();

        // Must be called before start()
        void setMaxQueueSize(int maxSize)
        {
            this->maxQueueSize_ = maxSize;
        }
        void setThreadInitCallback(const Task& cb)
        {
            threadInitCallback_ = cb;
        }

        void start(int numThreads); // 初始化线程数目
        void stop();

        const string& name() const
        {
            return name_;
        }

        size_t queueSize() const;

        void run(Task f);   // 将任务f加入线程池运行

    private:
        bool isFull() const REQUIRES(mutex_);
        void runInThread(); // 线程创建时所传入的运行函数: --> take( )--> f( ) 即运行run中添加的任务f
        Task take();    // 从queue中获取任务

        mutable MutexLock mutex_;
        // 用来告知线程池里面的线程当前是否有尚未执行的任务（即任务列表是否空）
        Condition notEmpty_ GUARDED_BY(mutex_);
        // 用来告知传递任务的线程当前线程池的任务列表是否已经塞满
        Condition notFull_ GUARDED_BY(mutex_);
        string name_;
        Task threadInitCallback_;
        vector<unique_ptr<muduo::Thread> > threads_;
        deque<Task> queue_ GUARDED_BY(mutex_);
        size_t maxQueueSize_;
        bool running_;
    };
}


#endif //MYMUDUO_THREADPOOL_H
