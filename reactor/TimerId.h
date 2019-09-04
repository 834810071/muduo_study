//
// Created by jxq on 19-8-26.
//

#ifndef MYMUDUO_TIMERID_H
#define MYMUDUO_TIMERID_H

#include "../base/copyable.h"
#include "Timer.h"

namespace muduo {

class Timer;

///
/// An opaque identifier, for canceling Timer.
///
class TimerId : public muduo::copyable {
public:
     TimerId(Timer* timer, int64_t seq = 0)
        : timer_(timer),
          sequence_(seq)
    {

    }

    friend class TimerQueue;
private:
    Timer* timer_;
    int64_t sequence_;
};

}

#endif //MYMUDUO_TIMERID_H
