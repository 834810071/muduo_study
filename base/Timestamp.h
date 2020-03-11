//
// Created by jxq on 19-6-15.
//

#ifndef MYMUDUO_TIMESTAMP_H
#define MYMUDUO_TIMESTAMP_H

#include <boost/operators.hpp>

#include "copyable.h"
#include "Types.h"

namespace muduo
{
class Timestamp : public muduo::copyable,
                  public boost::equality_comparable<Timestamp>,
                  public boost::less_than_comparable<Timestamp>
{
public:
    // 构造一个非法的TimeStamp类
    Timestamp()
        : microSecondsSinceEpoch_(0)
    {

    }

    // 构造一个指定时间的Timestamp类
    // explicit关键字的作用就是防止类构造函数的隐式自动转换.
    explicit Timestamp(int64_t microSecondsSinceEpochArg)
        : microSecondsSinceEpoch_(microSecondsSinceEpochArg)
    {

    }

    void swap(Timestamp& that)
    {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }

    // const指明了这个函数不会修改该类的任何成员数据的值，称为常量成员函数。
    string toString() const;
    string toFormattedString(bool showMicroseconds = true) const;

    bool valid() const { return microSecondsSinceEpoch_ > 0; }

    // for internal usage.
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    time_t secondsSinceEpoch() const    // time_t == long int
    {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }

    // get time of now;
    static Timestamp now();
    static  Timestamp invalid()
    {
        return Timestamp();
    }

    static Timestamp fromUnixTime(time_t t)
    {
        return fromUnixTime(t, 0);
    }

    static Timestamp fromUnixTime(time_t t, int microseconds)
    {
        return Timestamp(static_cast<int64_t >(t) * kMicroSecondsPerSecond + microseconds);
    }

    static const int kMicroSecondsPerSecond = 1000 * 1000;  // 转换成毫秒
private:
    int64_t microSecondsSinceEpoch_;
};

// 引入内联函数的主要目的是：解决程序中函数调用的效率问题
inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

// 返回时间戳之差  秒为单位
inline double timeDifference (Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast< double >(diff) / Timestamp::kMicroSecondsPerSecond;
}

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}

}



#endif //MYMUDUO_TIMESTAMP_H
































