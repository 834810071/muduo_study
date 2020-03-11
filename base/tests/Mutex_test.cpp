//
// Created by jxq on 19-6-24.
//

#include "../Mutex.h"
#include "../CountDownLatch.h"
#include "../Thread.h"
#include "../Timestamp.h"

#include <vector>
#include <stdio.h>

using namespace std;
using namespace muduo;

MutexLock g_mutex;
vector<int> g_vec;
const int kCount = 10 * 1000 * 1000;

void threadFunc()
{
    for (int i = 0; i < kCount; ++i)
    {
        MutexLockGuard lock(g_mutex);
        g_vec.push_back(i);
    }
}

int foo() __attribute__ ((noinline));   // fun函数不能作为inline函数优化

int g_count = 0;

int foo()
{
    MutexLockGuard lock(g_mutex);
    if (!g_mutex.isLockedByThisThread())
    {
        printf("FAIL\n");
        return -1;
    }
    ++g_count;
    return 0;
}

int main(int argc, char** argv)
{
    MCHECK(foo());
    if (g_count != 1)
    {
        printf("MCHECK calls twice.\n");
        abort();
    }

    const int kMaxThreads = 8;
    g_vec.reserve(kMaxThreads * kCount);

    Timestamp start(Timestamp::now());
    for (int i = 0; i < kCount; ++i)
    {
        g_vec.push_back(i);
    }

    printf("single thread without lock %f\n", timeDifference(Timestamp::now(), start));

    start = Timestamp::now();
    threadFunc();
    printf("single thread with lock %f\n", timeDifference(Timestamp::now(), start));

    for (int nthreads = 1; nthreads < kMaxThreads; ++nthreads)
    {
        vector<unique_ptr<Thread>> threads; // 某个时刻只能有一个unique_ptr指向一个给定的对象。
                                            // 当unique_ptr被销毁时，它所指向的对象也被销毁。uniptr_ptr表达的是一种独占的思想。
        g_vec.clear();
        start = Timestamp::now();
        for (int i = 0; i < nthreads; ++i)
        {
            threads.emplace_back(new Thread(&threadFunc));
            threads.back()->start();
        }
        printf("join......\n");
        for (int i = 0; i < nthreads; ++i)
        {
            threads[i] -> join();
        }

        printf("%d thread(s) with lock %f\n", nthreads, timeDifference(Timestamp::now(), start));
    }
    return 0;
}