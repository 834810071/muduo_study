//
// Created by jxq on 19-8-22.
//

#include "../EventLoop.h"
#include <iostream>

using namespace std;

muduo::EventLoop* g_loop;

void threadFunc()
{
    g_loop->loop();
}

int main()
{
    muduo::EventLoop loop;
    g_loop = &loop;

    muduo::Thread t(threadFunc);
    t.start();
    t.join();
}