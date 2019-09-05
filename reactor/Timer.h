//
// Created by jxq on 19-8-26.
//

#ifndef MYMUDUO_TIMER_H
#define MYMUDUO_TIMER_H

#include <boost/core/noncopyable.hpp>
#include "../base/Timestamp.h"
#include "Callbacks.h"
#include "../base/Atomic.h"

namespace muduo
{

///
/// Internal class for timer event.
///
class Timer : boost::noncopyable
{
public:
    Timer(const TimerCallback& cb, Timestamp when, double interval)
        : callback_(cb),
        expiration_(when),
        interval_(interval),
        repeat_(interval > 0.0),
        sequence_(s_numCreated_.incrementAndGet())
    {

    }

    void run() const
    {
        callback_();
    }

    Timestamp expiration() const
    {
        return expiration_;
    }

    bool repeat() const
    {
        return repeat_;
    }

    int64_t sequence() const { return sequence_; }

    void restart(Timestamp now);
private:
    const TimerCallback callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static AtomicInt64 s_numCreated_;
};

}

#endif //MYMUDUO_TIMER_H
