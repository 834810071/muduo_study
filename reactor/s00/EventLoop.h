//
// Created by jxq on 19-6-12.
//

#ifndef MYMUDUO_EVENTLOOP_H
#define MYMUDUO_EVENTLOOP_H


#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include "../../base/Thread.h"
#include "../s02/TimerId.h"
//#include "../s02/TimerQueue.h"
//#include "../s01/Channel.h"
//#include "../s01/Poller.h"
#include <vector>


namespace muduo
{

class Poller;
class Channel;
class TimerQueue;

class EventLoop : boost::noncopyable{   // 允许程序轻松实现一个不可复制的类。
public:
    typedef boost::function<void()> Functor;
    EventLoop();    // 构造函数
    ~EventLoop();   // 析构函数

    void loop();    // 成员函数

    // 断言是否在当前线程
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

    static EventLoop* getEventLoopOfCurrentThread();

    void quit();

    // internal use only
    void wakeup();
    void updateChannel(Channel* channel);

    // timers

    ///
    /// Runs callback at 'time'.
    ///
    TimerId runAt(const Timestamp& time, const TimerCallback& cb);
    ///
    /// Runs callback after @c delay seconds.
    ///
    TimerId runAfter(double delay, const TimerCallback& cb);
    ///
    /// Runs callback every @c interval seconds.
    ///
    TimerId runEvery(double interval, const TimerCallback& cb);

    void runInLoop(const Functor& cb);

    void queueInLoop(const Functor& cb);
private:

    void abortNotInLoopThread();
    void handleRead();  // waked up
    void doPendingFunctors();

    typedef std::vector<Channel*> ChannelList;

    bool looping_;                  // atomic
    bool quit_;                     // atomic
    bool callingPendingFunctors_;   // atomic
    const pid_t threadId_;     // 创建进程时经常会用到进程号的类型定义:pid_t int类型
    boost::scoped_ptr<Poller> poller_;
    boost::scoped_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;
    // unlike in TimerQueue, which is an internal class,
    // we don't expose Channel to client.
    boost::scoped_ptr<Channel> wakeupChannel_;
    ChannelList activeChannels_;
    MutexLock mutex_;
    std::vector<Functor> pendingFunctors_;  // @GuardedBy mutex_


};

}


#endif //MYMUDUO_EVENTLOOP_H
