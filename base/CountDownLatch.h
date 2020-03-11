//
// Created by jxq on 19-6-21.
//

#ifndef MYMUDUO_COUNTDOWNLATCH_H
#define MYMUDUO_COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"

namespace muduo
{
    // 倒计时计数器
    class CountDownLatch : noncopyable
    {
    public:

        explicit CountDownLatch(int count);

        void wait();

        void countDown();

        int getCount() const;

    private:
        mutable MutexLock mutex_;   // 被mutable修饰的变量，将永远处于可变的状态，即使在一个const函数中
        // GUARDED_BY是一个应用在数据成员上的属性，它声明了数据成员被给定的监护权保护。
        // 对于数据的读操作需要共享的访问权限，而写操作需要独占的访问权限。
        Condition condition_ GUARDED_BY(mutex_);
        int count_ GUARDED_BY(mutex_);

    };
}

#endif //MYMUDUO_COUNTDOWNLATCH_H
