//
// Created by jxq on 19-7-1.
//
#include "../ThreadPool.h"
#include "../CountDownLatch.h"
#include "../CurrentThread.h"
// #include "../Logging.h"

#include <stdio.h>
#include <unistd.h> // usleep
#include <iostream>

using namespace std;

void print()
{
    printf("tid = %d\n", muduo::CurrentThread::tid());
}

void printString(const string& str)
{
    //LOG_INFO << str;
    cout << str << endl;
    usleep(100*1000);
}

void test(int maxSize)
{
    //LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
    cout << "Test ThreadPool with max queue size = " << maxSize << endl;
    muduo::ThreadPool pool("MainThreadPool");
    pool.setMaxQueueSize(maxSize);  // 设置任务队列大小
    pool.start(5);  // 线程池开启5个线程

    //LOG_WARN << "Adding";
    cout  << "Adding" << endl;
    pool.run(print);    // 添加任务
    pool.run(print);    // 添加任务
    for (int i = 0; i < 100; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof buf, "task %d", i);
        pool.run(bind(printString, string(buf)));   // 添加任务
    }
    // LOG_WARN << "Done";
    cout << "Done" << endl;

    muduo::CountDownLatch latch(1);
    pool.run(bind(&muduo::CountDownLatch::countDown, &latch));
    latch.wait();
    pool.stop();
}

int main()
{
    test(0);
    test(1);
    test(5);
    test(10);
    test(50);
}























