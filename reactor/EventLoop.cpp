//
// Created by jxq on 19-6-12.
//

#include "EventLoop.h"
#include "Poller.h"
#include "Channel.h"
#include "../base/Logging.h"
#include "TimerQueue.h"
#include "EPoller.h"



#include <assert.h>
#include <boost/bind.hpp>
#include <sys/eventfd.h>
#include <signal.h>
#include <iostream>

using namespace muduo;

__thread EventLoop* t_loopInThisThread = 0; // 每一个线程有一份独立实体，各个线程的值互不干扰
const int kPollTimeMs = 1000000;

static int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN); // 对 server 来说，为了不被SIGPIPE信号杀死，那就需要忽略SIGPIPE信号
    }
};

IgnoreSigPipe initObj;

// IO线程
EventLoop::EventLoop()
    : looping_(false),
    quit_(false),
    threadId_(CurrentThread::tid()), // 记住本对象所属线程
    //poller_(new Poller(this)),
    poller_(new EPoller(this)),
    timerQueue_(new TimerQueue(this)),  // 注册定时器事件
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this, wakeupFd_))
{
    LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;

    if (t_loopInThisThread) // 检查当前线程是否创建了其他EventLoop对象
    {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread // 终止线程
        << " exists in this thread " << threadId_;
    } else {
        t_loopInThisThread = this;
    }
    wakeupChannel_->setReadCallback(boost::bind(&EventLoop::handleRead, this));
    // we are always reading the wakeupfd
    wakeupChannel_->enableReading();
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
    assertInLoopThread();   // 保证事件循环在IO线程执行
    looping_ = true;
    quit_ = false;

    LOG_TRACE << "EventLoop " << this << " start loopig";
    //::poll(NULL, 0, 5*1000);    // 等5秒就退出
    while (!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);       // 这里会循环一定时间，通过唤醒wakefd_来执行doPendingFunctors();
        for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
        {
            (*it)->handleEvent(pollReturnTime_);   // 调用对应的回调函数
        }
        doPendingFunctors();    //处理用户在其他线程注册给IO线程的事件
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_  = " << threadId_
              << ", current thread id = " << CurrentThread::tid();
}

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

void EventLoop::quit()
{
    quit_ = true;
    // 如果在非当前IO线程调用,即创建EventLoop的线程中
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->removeChannel(channel);
}

TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
    return timerQueue_->addTimer(cb, time, 0.0);
}

TimerId EventLoop::runAfter(double delay, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, cb);
}

TimerId EventLoop::runEvery(double interval, const TimerCallback& cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(cb, time, interval);
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(cb);
    }
//    if (callingPendingFunctors_)
//    {
//        std::cout << "true" << std::endl;
//        std::cout << isInLoopThread() << std::endl;
//    }
//    else
//    {
//        std::cout << "false" << std::endl;
//        std::cout << isInLoopThread() << std::endl;
//    }
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();   // 目前而言是针对于TimerQueue而言
    }
}

void EventLoop::runInLoop(const Functor& cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i)
    {
        functors[i](); // Functor有可能再次调用queueInLoop(), 防止死锁
    }
    callingPendingFunctors_ = false;
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::cancel(TimerId timerId)
{
    return timerQueue_->cancel(timerId);
}

































