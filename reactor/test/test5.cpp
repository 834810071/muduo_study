//
// Created by jxq on 19-8-27.
//

#include "../EventLoop.h"
#include <stdio.h>

muduo::EventLoop* g_loop;

void print()
{
    printf("tid2 = %o\n", muduo::CurrentThread::tid());
}

void threadFunc()
{
    printf("tid1 = %o\n", muduo::CurrentThread::tid());
    g_loop->runAfter(1.0, print);
}

int main()
{
    muduo::EventLoop loop;
    g_loop = &loop;
    printf("tid0 = %o\n", muduo::CurrentThread::tid());
    muduo::Thread t(threadFunc);
    t.start();
    loop.loop();    // 循环

}