//
// Created by jxq on 19-8-28.
//

#include "../EventLoop.h"
#include "../../base/Thread.h"

#include <boost/bind.hpp>

#include <stdio.h>
#include <unistd.h>

using namespace muduo;

int cnt = 0;
EventLoop* g_loop;

void printTid()
{
    printf("pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
    printf("now %s\n", Timestamp::now().toString().c_str());
}

void print(const char* msg)
{
    printf("msg %s %s\n", Timestamp::now().toString().c_str(), msg);
    if (++cnt == 20)
    {
        g_loop->quit();
    }
}

void cancel(TimerId timer)
{
   // g_loop->cancel(timer);
    printf("cancelled at %s\n", Timestamp::now().toString().c_str());
}

int main()
{
    printTid();
    sleep(1);
    {
        EventLoop loop;
        g_loop = &loop;

        print("main");
        //1s之后调用print函数
        loop.runAfter(1, boost::bind(print, "once1"));
        loop.runAfter(1.5, boost::bind(print, "once1.5"));
        loop.runAfter(2.5, boost::bind(print, "once2.5"));
        loop.runAfter(3.5, boost::bind(print, "once3.5"));
        TimerId t45 = loop.runAfter(4.5, boost::bind(print, "once4.5"));
        //还没到4.5s,4.2s时调用了cancel,注销4.5s的那个定时器
        loop.runAfter(4.2, boost::bind(cancel, t45));
        loop.runAfter(4.8, boost::bind(cancel, t45));
        //每隔2s运行一下
        loop.runEvery(2, boost::bind(print, "every2"));
        //每隔3s运行一下
        TimerId t3 = loop.runEvery(3, boost::bind(print, "every3"));
        //到9.001s后，把每隔3s的定时器取消掉
        loop.runAfter(9.001, boost::bind(cancel, t3));

        loop.loop();
        print("main loop exits");
    }
}