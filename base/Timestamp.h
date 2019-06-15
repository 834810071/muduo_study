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







private:
    int64_t microSecondsSinceEpoch_;
};
}



#endif //MYMUDUO_TIMESTAMP_H
































