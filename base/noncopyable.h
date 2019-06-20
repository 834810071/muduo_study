//
// Created by jxq on 19-6-20.
//

#ifndef MYMUDUO_NONCOPYABLE_H
#define MYMUDUO_NONCOPYABLE_H

namespace muduo
{
    class noncopyable
    {
    public:
        noncopyable(const noncopyable&) = delete;        // 禁止使用该函数
        void operator=(const noncopyable&) = delete;     // 禁止使用该函数

    protected:
        noncopyable() = default;
        ~noncopyable() = default;
    };
}

#endif //MYMUDUO_NOCOPYABLE_H
