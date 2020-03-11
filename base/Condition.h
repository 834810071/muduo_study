//
// Created by jxq on 19-6-21.
//

#ifndef MYMUDUO_CONDITION_H
#define MYMUDUO_CONDITION_H


#include "Mutex.h"

namespace muduo
{
    class Condition : noncopyable {
    public:
        explicit Condition(MutexLock& mutex)
            : mutex_(mutex)
        {
            // 函数pthread_cond_init（）被用来初始化一个条件变量
            MCHECK(pthread_cond_init(&pcond_, NULL));
        }

        ~Condition()
        {
            MCHECK(pthread_cond_destroy(&pcond_));
        }

        void wait()
        {
            MutexLock::UnassignGuard ug(mutex_);
            // pthread_cond_wait() 用于阻塞当前线程，等待别的线程使用pthread_cond_signal()或pthread_cond_broadcast来唤醒它
            MCHECK(pthread_cond_wait(&pcond_, mutex_.getPthreadMutex()));
        }

        // returns true if time true, false otherwise;
        bool waitForSeconds(double seconds);

        void notify()
        {
            MCHECK(pthread_cond_signal(&pcond_));
        }

        void notifyAll()
        {
            MCHECK(pthread_cond_broadcast(&pcond_));
        }



    private:
        MutexLock& mutex_;
        pthread_cond_t pcond_;  // 条件变量
    };
}


#endif //MYMUDUO_CONDITION_H

























