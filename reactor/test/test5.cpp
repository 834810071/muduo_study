//
// Created by jxq on 19-8-27.
//

#include "../EventLoop.h"
#include <stdio.h>

muduo::EventLoop* g_loop;

void print()
{
    printf("tid = %o\n", muduo::CurrentThread::tid());
}

void threadFunc()
{
    g_loop->runAfter(1.0, print);
}

int main()
{
    muduo::EventLoop loop;
    g_loop = &loop;

    muduo::Thread t(threadFunc);
    t.start();
    loop.loop();    // 循环

}