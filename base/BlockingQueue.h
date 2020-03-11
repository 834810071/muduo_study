//
// Created by jxq on 19-6-24.
//

#ifndef MYMUDUO_BLOCKINGQUEUE_H
#define MYMUDUO_BLOCKINGQUEUE_H

#include "Condition.h"
#include "Mutex.h"

#include <deque>
#include <assert.h>

namespace muduo {
    template<typename T>
    class BlockingQueue : noncopyable {
    public:
        BlockingQueue()
                : mutex_(),
                  notEempty_(mutex_),
                  queue_() {

        }

        void put(const T &x) {
            MutexLockGuard lock(mutex_);
            queue_.push_back(x);
            notEempty_.notify();
        }

        void put(T &&x) {
            MutexLockGuard lock(mutex_);
            queue_.push_back(std::move(x));
            notEempty_.notify();
        }

        T take() {
            MutexLockGuard lock(mutex_);
            while (queue_.empty()) {
                notEempty_.wait();
            }

            assert(!queue_.empty());
            T front(std::move(queue_.front()));
            queue_.pop_front();
            return std::move(front);
        }

        size_t size() const {
            MutexLockGuard lock(mutex_);
            return queue_.size();
        }

    private:
        mutable MutexLock mutex_;
        Condition notEempty_ GUARDED_BY(mutex_);
        std::deque<T> queue_ GUARDED_BY(mutex_);
    };
}   // namespace muduo;

#endif //MYMUDUO_BLOCKINGQUEUE_H
