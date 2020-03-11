//
// Created by jxq on 19-6-24.
//

#ifndef MYMUDUO_BOUNDEDBLOCKINGQUEUE_H
#define MYMUDUO_BOUNDEDBLOCKINGQUEUE_H

#include "Condition.h"
#include "Mutex.h"

#include <boost/circular_buffer.hpp>
#include <assert.h>

namespace muduo
{
    template <typename T>
    class BoundedBlockingQueue : noncopyable
    {
    public:
        explicit BoundedBlockingQueue(int maxsize)
            : mutex_(),
              notEmpty_(mutex_),
              notFull_(mutex_),
              queue_(maxsize)
        {

        }

        void put(const T& x)
        {
            MutexLockGuard lock(mutex_);
            while (queue_.full())
            {
                notFull_.wait();
            }
            assert(!queue_.full());
            queue_.push_back(x);
            notEmpty_.notify();
        }

        void put(T&& X)
        {
            MutexLockGuard lock(mutex_);
            while (queue_.full())
            {
                notFull_.wait();
            }
            assert(!queue_.full());
            queue_.push_back(std::move(X));
            notEmpty_.notify();
        }

        T take()
        {
            MutexLockGuard lock(mutex_);
            while (queue_.empty())
            {
                notEmpty_.wait();
            }
            assert(!queue_.empty());
            T front(std::move(queue_.front()));
            queue_.pop_front();
            notFull_.notify();
            return std::move(front);
        }

        bool empty() const
        {
            MutexLockGuard lock(mutex_);
            return queue_.empty();
        }

        bool full() const
        {
            MutexLockGuard lock(mutex_);
            return queue_.full();
        }

        size_t size() const
        {
            MutexLockGuard lock(mutex_);
            return queue_.size();
        }

        size_t capacity() const
        {
            MutexLockGuard lock(mutex_);
            return queue_.capacity();
        }
    private:
        mutable MutexLock mutex_;
        Condition notEmpty_ GUARDED_BY(mutex_);
        Condition notFull_ GUARDED_BY(mutex_);
        boost::circular_buffer<T> queue_ GUARDED_BY(mutex_);
    };
}   // namespace muduo

#endif //MYMUDUO_BOUNDEDBLOCKINGQUEUE_H
