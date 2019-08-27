//
// Created by jxq on 19-8-27.
//

#ifndef MYMUDUO_EVENTLOOPTHREAD_H
#define MYMUDUO_EVENTLOOPTHREAD_H


#include <boost/core/noncopyable.hpp>
#include "../s00/EventLoop.h"

namespace muduo
{
class EventLoopThread : boost::noncopyable{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();

private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    MutexLock mutex_;
    Condition cond_;
};
}


#endif //MYMUDUO_EVENTLOOPTHREAD_H
