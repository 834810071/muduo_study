//
// Created by jxq on 19-6-21.
//

#include "CountDownLatch.h"

using namespace muduo;

// 倒计时计数器
CountDownLatch::CountDownLatch(int count)
    : mutex_(),
      condition_(mutex_),
      count_(count)
{

}

void CountDownLatch::wait()
{
    MutexLockGuard lock(mutex_);    // 防止忘记给mutex解锁
    while (count_ > 0)
    {
        condition_.wait();
    }
}

void CountDownLatch::countDown()
{
    MutexLockGuard lock(mutex_);
    --count_;
    if (count_ == 0)
    {
        condition_.notifyAll();
    }
}

int CountDownLatch::getCount() const
{
    MutexLockGuard lock(mutex_);
    return count_;
}