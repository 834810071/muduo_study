//
// Created by jxq on 19-8-22.
//

#include "../EventLoop.h"
#include <iostream>

using namespace std;

void threadFunc()
{
    printf("threadFunc(): pid = %d, tid = %d\n",
            getpid(), muduo::CurrentThread::tid());
    muduo::EventLoop loop;
    loop.loop();
}

int main()
{
    // getpid返回当前进程标识
    // tid()线程id
    printf("main(): pid = %d, tid = %d\n",
           getpid(), muduo::CurrentThread::tid());
    muduo::EventLoop loop;

    muduo::Thread thread(threadFunc);
    thread.start();

    loop.loop();
    pthread_exit(NULL);
}