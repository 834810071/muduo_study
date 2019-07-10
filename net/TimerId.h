//
// Created by jxq on 19-7-10.
//

#ifndef MYMUDUO_TIMERID_H
#define MYMUDUO_TIMERID_H

#include "../base/copyable.h"

namespace muduo
{
    namespace net
    {
        class Timer;

        class TimerId : public muduo::copyable
        {
        public:
            TimerId()
                : timer_(NULL),
                  sequence_(0)
            {

            }

            TimerId(Timer* timer, int64_t seq)
                : timer_(timer),
                  sequence_(seq)
            {

            }

            friend class TimerQueue;

        private:
            Timer* timer_;
            int64_t sequence_;
        };

    }   // namespace net
}   // namespace muduo

#endif //MYMUDUO_TIMERID_H
