//
// Created by jxq on 19-9-3.
//

#ifndef MYMUDUO_EVENTLOOPTHREADPOLL_H
#define MYMUDUO_EVENTLOOPTHREADPOLL_H


#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include "EventLoop.h"
#include "EventLoopThread.h"

namespace muduo
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
public:
    EventLoopThreadPool(EventLoop* loop);
    ~EventLoopThreadPool();
    void setThreadNum(int numThreads)
    {
        numThreads_ = numThreads;
    }
    void start();
    EventLoop* getNextLoop();

private:
    EventLoop* baseLoop_;
    bool started_;
    int numThreads_;
    int next_;  // always in loop thread
    boost::ptr_vector<EventLoopThread> threads_;
    std::vector<EventLoop*> loops_;
};

}


#endif //MYMUDUO_EVENTLOOPTHREADPOLL_H
