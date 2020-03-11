//
// Created by jxq on 19-6-23.
//

#include "../CurrentThread.h"
#include "../Mutex.h"
#include "../Thread.h"
#include "../Timestamp.h"

#include <map>
#include <string>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

muduo::MutexLock g_mutex;   // 互斥锁，封装互斥器的创建与销毁
std::map<int, int> g_delays;

void threadFunc()
{
    // printf("tid = %d\n", muduo::CurrentThread::tid());
}

void threadFunc2(muduo::Timestamp start)
{
    muduo::Timestamp now(muduo::Timestamp::now());
    int delay = static_cast<int>(timeDifference(now, start));
    muduo::MutexLockGuard lock(g_mutex);
    ++g_delays[delay];
    // 自动解析释放锁
}

void forkBench()
{
    sleep(10);
    muduo::Timestamp start(muduo::Timestamp::now());
    const int kProcesses = 10 * 1000;

    for (int i = 0; i < kProcesses; ++i)
    {
        pid_t child = fork();   // -1表示失败
        if (child == 0)
        {
            exit(0);
        }
        else
        {
            waitpid(child, NULL, 0);
        }
    }

    double timeUsed = timeDifference(muduo::Timestamp::now(), start);
    printf("process creation time used %f us\n", timeUsed*1000000/kProcesses);
    printf("number of created processes %d\n", kProcesses);
}

int main(int argc, char** argv)
{
    printf("pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());
    muduo::Timestamp start(muduo::Timestamp::now());

    const int kThread = 100 * 1000;
    for (int i = 0; i < kThread; ++i)
    {
        muduo::Thread t1(threadFunc);
        t1.start();
        t1.join();
    }

    double timeUsed = timeDifference(muduo::Timestamp::now(), start);
    printf("thread creation time %f us\n", timeUsed * 1000000 / kThread);
    printf("number of created threads %d\n", muduo::Thread::numCreated());

    for (int i = 0; i < kThread; ++i)
    {
        muduo::Timestamp now(muduo::Timestamp::now());
        muduo::Thread t2(std::bind(threadFunc2, now));
        t2.start();
        t2.join();
    }
    {
        muduo::MutexLockGuard lock(g_mutex);
        for (const auto& delay : g_delays)
        {
            printf("delay = %d, count = %d\n",
                   delay.first, delay.second);
        }
        // 自动解析释放锁
    }

    forkBench();

    return 0;
}


























