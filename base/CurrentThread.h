//
// Created by jxq on 19-6-20.
//

#ifndef MYMUDUO_CURRENTTHREAD_H
#define MYMUDUO_CURRENTTHREAD_H

#include "Types.h"

namespace muduo
{

    namespace CurrentThread
    {
        // 1. __thread 关键字表示每一个线程有一份独立的实体，每一个线程都不会干扰。

        // 2. __thread 只能修饰POD变量，简单的来说可以是如下几种变量

        // (1) 基本类型 (int , float 等等)

        // (2)指针类型

        // (3) 不带自定义构造函数和析构函数的类，如果希望修饰带自定义构造和析构函数的类，需要用到指针。

        // internal
        extern __thread int t_cachedTid;
        extern __thread char t_tidString[32];
        extern __thread int t_tidStringLength;
        extern __thread const char* t_threadName;
        void cacheTid();

        // 在c/c++中，为了解决一些频繁调用的小函数大量消耗栈空间（栈内存）的问题，
        // 特别的引入了inline修饰符，表示为内联函数。
        inline int tid()
        {
            if (__builtin_expect(t_cachedTid == 0, 0))
            {
                cacheTid();
            }
            return t_cachedTid;
        }

        inline const char* tidString()  // for logging
        {
            return t_tidString;
        }

        inline int tidStringLength() // for logging
        {
            return t_tidStringLength;
        }

        bool isMainThread();

        void sleepUsec(int64_t usec);   // for testing

        string stackTrace(bool demangle);

    }   // namespace CurrentThread

}   // namespace muduo



#endif //MYMUDUO_CURRENTTHREAD_H
