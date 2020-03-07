//
// Created by jxq on 19-8-26.
//

#include <boost/bind.hpp>
#include "TimerQueue.h"
#include <sys/timerfd.h>
#include "../base/Logging.h"
#include <iostream>
#include <boost/foreach.hpp>

using namespace std;


namespace muduo
{
    namespace detail
    {
        int createTimerfd()
        {
            int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                           TFD_NONBLOCK | TFD_CLOEXEC); // 它是用来创建一个定时器描述符timerfd
            if (timerfd < 0)
            {
                LOG_SYSFATAL << "Failed in timerfd_create";
            }
            return timerfd;
        }

        struct timespec howMuchTimeFromNow(Timestamp when)
        {
            int64_t microseconds = when.microSecondsSinceEpoch()
                                   - Timestamp::now().microSecondsSinceEpoch();
            if (microseconds < 100)
            {
                microseconds = 100;
            }
            struct timespec ts;
            ts.tv_sec = static_cast<time_t>(
                    microseconds / Timestamp::kMicroSecondsPerSecond);
            ts.tv_nsec = static_cast<long>(
                    (microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
            return ts;
        }

        void readTimerfd(int timerfd, Timestamp now)
        {
            uint64_t howmany;
            // 当定时器超时，read读事件发生即可读，返回超时次数
            ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
            LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
            if (n != sizeof howmany)
            {
                LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
            }
        }

        void resetTimerfd(int timerfd, Timestamp expiration)
        {
            // wake up loop by timerfd_settime()
            struct itimerspec newValue;
            struct itimerspec oldValue;
            bzero(&newValue, sizeof newValue);
            bzero(&oldValue, sizeof oldValue);
            // 结构体itimerspec就是timerfd要设置的超时结构体，
            // 它的成员it_value表示定时器第一次超时时间
            newValue.it_value = howMuchTimeFromNow(expiration);
            // 启动或关闭有fd指定的定时器
            int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);  // 能够启动和停止定时器
            if (ret)
            {
                LOG_SYSERR << "timerfd_settime()";
            }

        }
    }
}

using namespace muduo;
using namespace muduo::detail;

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(detail::createTimerfd()),
      timerfdChannel_(loop, timerfd_),  // 定时器描述符所属的Channel
      timers_(),
      callingExpiredTimers_(false)
{
    timerfdChannel_.setReadCallback(
            boost::bind(&TimerQueue::handleRead, this));
    // we are always reading the timerfd, we disarm it with timerfd_settime.
    timerfdChannel_.enableReading();    // 添加timefd_到Poller::pollfds_
}

TimerQueue::~TimerQueue()
{
    ::close(timerfd_);
    // do not remove channel, since we're in EventLoop::dtor();
    for (TimerList::iterator it = timers_.begin();
         it != timers_.end(); ++it)
    {
        delete it->second;
    }

}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    // 当定时器超时，read读事件发生即可读，返回超时次数
    detail::readTimerfd(timerfd_, now); // 读取timerfd_

    std::vector<Entry> expired = getExpired(now);

    callingExpiredTimers_ = true;
    cancelingTimers_.clear();
    // 执行到期任务
    // safe to callback outside critical section
    for (std::vector<Entry>::iterator it = expired.begin();
         it != expired.end(); ++it)
    {
        it->second->run();
    }
    callingExpiredTimers_ = false;
    reset(expired, now);    // 清空到期队列 并给timerfd_重新赋值 [timers_开头没有到期才给timerfd_赋值(注册)，没到该怎么办]
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    assert(timers_.size() == activeTimers_.size());
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));  // UINTPTR_MAX: uintptr_t 类型对象的最大值
    TimerList::iterator it = timers_.lower_bound(sentry);   // 返回第一个未到期的Timer
    assert(it == timers_.end() || now < it->first);
    std::copy(timers_.begin(), it, back_inserter(expired));
    timers_.erase(timers_.begin(), it);

    BOOST_FOREACH(Entry entry, expired)
    {
        ActiveTimer timer(entry.second, entry.second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert(n == 1); (void)n;
    }

    assert(timers_.size() == activeTimers_.size());
    return expired;
}

//TimerId TimerQueue::addTimer(const muduo::TimerCallback &cb, muduo::Timestamp when, double interval)
//{
//    Timer* timer = new Timer(cb, when, interval);
//    loop_->assertInLoopThread();
//    bool earlisestChanged = insert(timer);
//
//    if (earlisestChanged)   // 如果插入在开头，那么需要重置timerfd_
//    {
//        //cout << i << endl;
//        detail::resetTimerfd(timerfd_, timer->expiration());
//    }
//}

TimerId TimerQueue::addTimer(const TimerCallback& cb,
                 Timestamp when,
                 double interval)
{
    Timer* timer = new Timer(cb, when, interval);
    loop_->runInLoop(boost::bind(&TimerQueue::addTimerInLoop, this, timer));    // 在IO线程中执行添加timer操作
    return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    loop_->assertInLoopThread();
    bool earlisestChanged = insert(timer);

    if (earlisestChanged)   // 如果插入在开头，那么需要重置timerfd_
    {
        detail::resetTimerfd(timerfd_, timer->expiration());    // 重新启动定时器
    }
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;

    for (std::vector<Entry>::const_iterator it = expired.begin();
         it != expired.end(); ++it)
    {
        ActiveTimer timer(it->second, it->second->sequence());
        if (it->second->repeat()        // 如果是重复执行的任务
            && cancelingTimers_.find(timer) == cancelingTimers_.end())     // 并且在取消事件中没有找到  然后再重复调用
        {
            it->second->restart(now);
            insert(it->second);
        }
        else
        {
            // FIXME move to a free list
            delete it->second;
        }
    }

    if (!timers_.empty())
    {
        nextExpire = timers_.begin()->second->expiration(); // 判断timers_开头是否到期
    }

    if (nextExpire.valid()) // 如果没有到期
    {
        resetTimerfd(timerfd_, nextExpire); // 给timerfd_重新赋值
    }
}


bool TimerQueue::insert(Timer* timer) {
    bool earliestChanged = false;   // 判断是否插入timers_开头
    assert(timers_.size() == activeTimers_.size());
    Timestamp when = timer->expiration();   // 到期时间
    TimerList::iterator it = timers_.begin();
    if (it == timers_.end() || when < it->first) {
        // 最早到期的定时器发生改变
        earliestChanged = true;
    }

    {   std::pair<TimerList::iterator, bool> result =
            timers_.insert(std::make_pair(when, timer));
        assert(result.second); (void)result;
    }

    {
        std::pair<ActiveTimerSet::iterator, bool> result =
                activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
        assert(result.second); (void)result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}

void TimerQueue::cancel(TimerId timerId)
{
    loop_->runInLoop(
            boost::bind(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::cancelInLoop(muduo::TimerId timerId)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer_, timerId.sequence_);
    ActiveTimerSet::iterator it = activeTimers_.find(timer);
    if (it != activeTimers_.end())
    {
        size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
        assert(n == 1); (void)n;
        delete it->first; // FIXME: no delete please
        activeTimers_.erase(it);
    }
    else if (callingExpiredTimers_)
    {
        cancelingTimers_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}