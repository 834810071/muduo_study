//
// Created by jxq on 19-6-25.
//

#include "../BlockingQueue.h"
#include "../CountDownLatch.h"
#include "../Thread.h"
#include "../Timestamp.h"

#include <map>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace std;

class Bench
{
public:
    Bench(int numThreads)
        : latch_(numThreads)
    {
        threads_.reserve(numThreads);

        for (int i = 0; i < numThreads; ++i)
        {
            char name[32];
            snprintf(name, sizeof name, "work thread %d", i);
            threads_.emplace_back(new muduo::Thread(
                    bind(&Bench::threadFunc, this), muduo::string(name)
                    ));
        }

        for (auto& thr : threads_)
        {
            thr->start();
        }
    }

    void run(int times)
    {
        printf("waiting for count down latch\n");
        latch_.wait();
        printf("all threads started\n");
        for (int i = 0; i < times; ++i)
        {
            muduo::Timestamp now(muduo::Timestamp::now());
            queue_.put(now);
            usleep(1000);   // usleep()是以微秒级别为单位的
        }
    }

    void joinAll()
    {
        for (size_t i = 0; i < threads_.size(); ++i)
        {
            queue_.put(muduo::Timestamp::invalid());
        }

        for (auto& thr : threads_)
        {
            thr -> join();
        }
    }

private:
    void threadFunc()
    {
        printf("tid = %d, %s started\n",
                muduo::CurrentThread::tid(),
                muduo::CurrentThread::name());

        map<int, int> delays;
        latch_.countDown();
        bool running = true;
        while (running)
        {
            muduo::Timestamp t(queue_.take());
            muduo::Timestamp now(muduo::Timestamp::now());
            if (t.valid())
            {
                int delay = static_cast<int>(timeDifference(now, t) * 1000000);
                ++delays[delay];
            }
            running = t.valid();
        }

        printf("tid = %d, %s stoped\n",
               muduo::CurrentThread::tid(),
               muduo::CurrentThread::name());

        for (const auto& delay : delays)
        {
            printf("tid = %d, delay = %d, count = %d\n",
                    muduo::CurrentThread::tid(),
                    delay.first,
                    delay.second);
        }
    }

    muduo::BlockingQueue<muduo::Timestamp> queue_;
    muduo::CountDownLatch latch_;
    vector<unique_ptr<muduo::Thread> > threads_;
};

int main(int argc, char** argv)
{
    int threads = 5;

    Bench t(threads);   // 创建threads个线程，并且绑定函数
    t.run(10000);
    t.joinAll();

    return 0;
}