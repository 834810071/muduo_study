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
    explicit TimerId(Timer* timer)
        : value_(timer)
    {

    }
private:
    Timer* value_;
};

}

#endif //MYMUDUO_TIMERID_H
