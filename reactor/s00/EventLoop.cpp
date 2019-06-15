//
// Created by jxq on 19-6-12.
//

#include "EventLoop.h"

#include "../../logging/Logging.h"

#include <assert.h>
#include <poll.h>

using namespace muduo;

__thread EventLoop* t_loopInThisThread = 0;

// IO线程
EventLoop::EventLoop()
    : looping_(false),
    threadId_(CurrentThread::tid()) // 记住本对象所属线程
{
    LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;

    if (t_loopInThisThread) // 检查当前线程是否创建了其他EventLoop对象
    {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread // 终止线程
        << " exists in this thread " << threadId_;
    } else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loopInThisThread = NULL;
}

// 主要功能是运行事件循环EventLoop::loop()
void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;

    ::poll(NULL, 0, 5*1000);

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_  = " << threadId_
              << ", current thread id = " << CurrentThread::tid();
}











