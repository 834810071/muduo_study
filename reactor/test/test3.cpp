//
// Created by jxq on 19-8-22.
//

#include <sys/timerfd.h>
#include "../EventLoop.h"
#include "../Poller.h"
#include "../Channel.h"
#include "../../base/Timestamp.h"
#include <time.h>
#include <signal.h>

//using namespace muduo;

muduo::EventLoop* g_loop;

void timeout(muduo::Timestamp t)
{
    printf("Timeout!\n");
    g_loop->quit();
}

int main()
{

    muduo::EventLoop loop;
    g_loop = &loop;

    // CLOCK_MONOTONIC:以固定的速率运行，从不进行调整和复位 ,它不受任何系统time-of-day时钟修改的影响
    // 函数创建一个定时器对象，同时返回一个与之关联的文件描述符
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    muduo::Channel channel(&loop, timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();    // 注册

    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 5;
    // timer_t a = *static_cast<timer_t>(&timerfd);
    // 此函数用于设置新的超时时间，并开始计时,能够启动和停止定时器;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);

    loop.loop();

    ::close(timerfd);
}
