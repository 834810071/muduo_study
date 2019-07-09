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
    // TimeZone for 1970~2030
    class TimeZone : public muduo::copyable
    {

    public:
        explicit TimeZone(const char* zonefile);
        // TimeZone(int eastOfUtc, const char* tzname)构造函数中，eastOfUtc表示UTC时间。中国内地的时间与UTC的时差为+8，也就是UTC+8
        TimeZone(int eastOfUtc, const char* tzname);
        TimeZone() = default;

        bool valid() const
        {
            return static_cast<bool>(data_);
        }

        struct tm toLocalTime(time_t secondsSinceEpoch) const;
        time_t fromLocalTime(const struct tm&) const;

        static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);
        static time_t fromUtcTime(const struct tm&);
        static time_t fromUtcTime(int year, int month, int day,
                                   int hour, int minute, int seconds);


        struct Data;
    private:
        // 智能指针
        std::shared_ptr<Data> data_;
    };

}

#endif //MYMUDUO_TIMEZONE_H
