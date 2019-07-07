//
// Created by jxq on 19-7-7.
//

#ifndef MYMUDUO_TIMEZONE_H
#define MYMUDUO_TIMEZONE_H

#include "copyable.h"
#include <memory>
#include <time.h>

namespace muduo
{

    class TimeZone : public muduo::copyable
    {

    public:
        explicit TimeZone(const char* zonefile);
        TimeZone(int eastOfUtc, const char* tzname);
        TimeZone() = default;

        bool valid() const
        {
            return static_cast<bool>(date_);
        }

        struct tm toLocalTime(time_t secondsSinceEpoch) const;
        time_t fromeLocalTime(const struct tm&) const;

        static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);
        static time_t fromUtcTime(const struct tm&);
        static time_t fromeUtcTime(int year, int month, int day,
                                   int hour, int minute, int seconds);

        struct Data;
    private:
        // 智能指针
        std::shared_ptr<Data> date_;
    };

}

#endif //MYMUDUO_TIMEZONE_H
