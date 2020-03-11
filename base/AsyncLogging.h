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
    class AsyncLogging {
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
            running_ = true;
            thread_.start();
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
        typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
        typedef BufferVector::value_type BufferPtr;

        const int flushInterval_;
        std::atomic<bool> running_;
        const string basename_;
        const off_t rollSize_;  // off_t用于文件 size_t用于对象
        muduo::Thread thread_;
        muduo::CountDownLatch latch_;
        muduo::MutexLock mutex_;
        muduo::Condition cond_ GUARDED_BY(mutex_);
        BufferPtr currentBuffer_ GUARDED_BY(mutex_);
        BufferPtr nextBuffer_ GUARDED_BY(mutex_);
        BufferVector buffers_ GUARDED_BY(mutex_);
    };
}   // namespace muduo



#endif //MYMUDUO_ASYNCLOGGING_H
