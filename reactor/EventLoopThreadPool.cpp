//
// Created by jxq on 19-9-3.
//

#include "EventLoopThreadPool.h"

using namespace muduo;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* loop)
    : baseLoop_(loop),
      started_(false),
      numThreads_(0),
      next_(0)
{

}
EventLoopThreadPool::~EventLoopThreadPool()
{
    // Don't delete loop, it's stack variable
}

void EventLoopThreadPool::start()
{
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < numThreads_; ++i)
    {
        EventLoopThread* t = new EventLoopThread;
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if (!loops_.empty())
    {
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}