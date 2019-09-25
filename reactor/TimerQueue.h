//
// Created by jxq on 19-8-26.
//

#ifndef MYMUDUO_TIMERQUEUE_H
#define MYMUDUO_TIMERQUEUE_H

#include <set>
#include "../base/Timestamp.h"
#include "Timer.h"
#include "EventLoop.h"
#include "Channel.h"
#include "TimerId.h"

namespace muduo {

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue {
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();


    ///
    /// Schedules the callback to be run at given time,
    /// repeats if @c interval > 0.0.
    ///
    /// Must be thread safe. Usually be called from other threads.
    TimerId addTimer(const TimerCallback& cb,
            Timestamp when,
            double interval);

    void cancel(TimerId timerId);




private:
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;
    typedef std::pair<Timer*, int64_t> ActiveTimer;
    typedef std::set<ActiveTimer> ActiveTimerSet;

    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    // called when timerfd alarms
    void handleRead();
    // move out all expired timers
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer* timer);

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    // Timer list sorted by expiration
    TimerList timers_;

    // for cancel()
    bool callingExpiredTimers_; /* atomic */
    ActiveTimerSet activeTimers_;   // 保存的是目前有效的Timer指针，并满足timers_.size() == activeTimers_.size()
    ActiveTimerSet cancelingTimers_;
};

}


#endif //MYMUDUO_TIMERQUEUE_H
