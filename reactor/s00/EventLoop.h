//
// Created by jxq on 19-6-12.
//

#ifndef MYMUDUO_EVENTLOOP_H
#define MYMUDUO_EVENTLOOP_H


#include <boost/noncopyable.hpp>
#include "../../base/Thread.h"

namespace muduo
{
class EventLoop : boost::noncopyable{   // 允许程序轻松实现一个不可复制的类。
public:
    EventLoop();    // 构造函数
    ~EventLoop();   // 析构函数

    void loop();    // 成员函数

    void assertInLoopThread()
    {
        if (!isInLoopThread())
        {
            abortNotInLoopThread();
        }
    }

    bool isInLoopThread() const
    {
        return threadId_ == CurrentThread::tid();
    }

private:

    void abortNotInLoopThread();

    bool looping_;
    const pid_t  threadId_;     // 创建进程时经常会用到进程号的类型定义:pid_t int类型
};

}


#endif //MYMUDUO_EVENTLOOP_H
