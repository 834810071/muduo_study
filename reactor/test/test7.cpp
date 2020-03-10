//
// Created by jxq on 19-8-27.
//

#include <iostream>
#include "../EventLoopThread.h"

using namespace std;

void runInThread()
{
    printf("runInThread(): pid = %d, tid = %d\n",
           getpid(), muduo::CurrentThread::tid());
}

int main()
{
    printf("main(): pid = %d, tid = %d\n",
            getpid(), muduo::CurrentThread::tid());

    muduo::EventLoopThread loopThread;
    muduo::EventLoop* loop = loopThread.startLoop();    // 线程2在loop循环

    // 主线程退出，子线程还没有退出
    loop->runInLoop(runInThread);   // 因为是主线程，所以放到queueInLoop
    sleep(1);
    loop->runAfter(2, runInThread);
    sleep(3);
    loop->quit();
    printf("exit main().\n");
    return 0;
}