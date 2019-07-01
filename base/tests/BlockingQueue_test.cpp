//
// Created by jxq on 19-6-25.
//
#include "../BlockingQueue.h"
#include "../CountDownLatch.h"
#include "../Thread.h"

#include <memory>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

using namespace std;

class Test {
public:
    Test(int numThreads)
        : latch_(numThreads)
    {
        for (int i = 0; i < numThreads; ++i)
        {
            char name[32];
            snprintf(name, sizeof name, "work thread %d", i);
            threads_.emplace_back(new muduo::Thread(
                    bind(&Test::threadFunc, this),
                    muduo::string(name)
                    ));
        }

        for (auto& thr : threads_)
        {
            thr -> start();
        }
    }

    void run(int times)
    {
        printf("waiting for count down latch\n");
        latch_.wait();
        // cout << "run: latch_:\t" << latch_.getCount() << endl;
        printf("all threads started\n");
        for (int i = 0; i < times; ++i)
        {
            char buf[32];
            snprintf(buf, sizeof buf, "hello %d", i);
            queue_.put(buf);
            printf("tid = %d, put data = %s, size = %zd\n", muduo::CurrentThread::tid(), buf, queue_.size());
        }
    }

    void joinAll()
    {
        for (size_t i = 0; i < threads_.size(); ++i) {
            queue_.put("stop");
        }

        for (auto& thr : threads_)
        {
            thr->join();
        }
    }



private:
    void threadFunc()
    {
        printf("tid = %d, %s started\n",
                muduo::CurrentThread::tid(),
                muduo::CurrentThread::name());

        latch_.countDown(); // 倒计时-1
        // cout << "latch_:\t" << latch_.getCount() << endl;
        bool running = true;
        while (running)
        {
            string d(queue_.take());
            printf("tid = %d, get data = %s, size = %zd\n", muduo::CurrentThread::tid(), d.c_str(), queue_.size());
            running = (d != "stop");
        }

        printf("tid = %d, %s stopped\n",
                muduo::CurrentThread::tid(),
                muduo::CurrentThread::name());

    }

    muduo::BlockingQueue<string> queue_;
    muduo::CountDownLatch latch_;   // 计时器
    vector<unique_ptr<muduo::Thread> > threads_;
};

void testMove()
{
    muduo::BlockingQueue<unique_ptr<int>> queue;
    queue.put(unique_ptr<int>(new int(42)));
    unique_ptr<int> x = queue.take();
    printf("took %d\n", *x);
    *x = 123;
    queue.put(std::move(x));
    std::unique_ptr<int> y = queue.take();
    printf("took %d\n", *y);
}

int main(int argc, char** argv)
{
    printf("pid = %d, tid = %d\n", ::getpid(), muduo::CurrentThread::tid());
    Test t(5);
    t.run(100);
    t.joinAll();

   //  testMove();

    return 0;
}





















