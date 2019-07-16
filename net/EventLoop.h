//
// Created by jxq on 19-7-10.
//

#ifndef MYMUDUO_EVENTLOOP_H
#define MYMUDUO_EVENTLOOP_H

#include <atomic>
#include <functional>
#include <vector>

#include <boost/any.hpp>

#include "../base/Mutex.h"
#include "../base/CurrentThread.h"
#include "../base/Timestamp.h"
#include "Callbacks.h"
#include "TimerId.h"

namespace muduo
{
    namespace net
    {
        class Channel;
        class Poller;
        class TimerQueue;

        ///
        /// Rector, at most one per thread.
        ///
        /// This is an interface class, so don't expose too much details;
        class EventLoop : noncopyable
        {
        public:
            typedef std::function<void()> Functor;

            EventLoop();
            ~EventLoop();   // force out-line dtor, for std::unique_ptr members.

            ///
            /// Loops forever
            ///
            /// Must be called in the same thread as creation of the object.
            ///
            void loop();

            /// Quits loop
            ///
            /// This is not 100% thread safe, if you call through a row pointer,
            /// better to call through shared_ptr<EventLoop> for 100% safety.
            void quit();

            ///
            /// Time when poll returns, usually means data arrival.
            ///
            Timestamp pollReturnTime() const
            {
                return pollReturnTime_;
            }

            int64_t iteration() const
            {
                return iteration_;
            }

            /// Runs callback immediately in the loop thread.
            /// It wakes up the loop, and run the cb. cb[回调函数]
            /// If in the same loop thread, cb is run within the function.
            /// Safe to call from other threads.
            void runInLoop(Functor cb);
            /// Queues callback in the loop thread.
            /// Runs after finish pooling.
            /// Safe to call from other threads.
            void queueInLoop(Functor cb);

            size_t queueSize() const;

            // timers

            ///
            /// Runs callback at 'time'
            /// Safe to call from other threads
            ///
            TimerId runAt(Timestamp time, TimerCallback cb);
            ///
            /// Runs callback after @c delay seconds.
            /// Safe to call from other threads.
            ///
            TimerId runAfter(double delay, TimerCallback cb);
            ///
            /// Runs callback after @c delay seconds
            /// Safe to call from other threads.
            ///
            TimerId runEvery(double interval, TimerCallback cb);
            ///
            /// Cancels the timer.
            /// Safe to call from other threads
            ///
            void cancel(TimerId timerId);

            // internal usage
            void wakeup();
            void updateChannel(Channel* channel);
            void removeChannel(Channel* channel);
            bool hasChannel(Channel* channel);

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

            bool eventHandling() const
            {
                return eventHandling_;  // bool
            }

            void setContext(const boost::any& context)
            {
                context_ = context;
            }

            const boost::any& getContext() const
            {
                return context_;
            }

            boost::any* getMutableContext() // 可变
            {
                return &context_;
            }

            static EventLoop* getEventLoopOfCurrentThread();

        private:
            void abortNotInLoopThread();
            void handleRead();  // waked up
            void doPendingFunctors();

            void printActiveChannels() const;   // DEBUG

            typedef std::vector<Channel*> ChannelList;

            bool looping_;  // atomic 当前是否在loop
            std::atomic<bool> quit_;    // 是否退出
            bool eventHandling_; // atomic  是否在处理事件
            bool callingPendingFunctors_;   // atomic   队列中是否有待处理函数
            int64_t iteration_; // 迭代器
            const pid_t threadId_;  // 线程ID
            Timestamp pollReturnTime_;  // 时间戳，poll返回的时间戳
            std::unique_ptr<Poller> poller_;    // poller对象
            std::unique_ptr<TimerQueue> timerQueue_;    // 定时器队列
            int wakeupFd_;  // 唤醒fd 用于eventfd，线程间通信

            // 与内部类TimerQueue不同，不向客户端公开通道。
            std::unique_ptr<Channel> wakeupChannel_;    // 唤醒通道 wakeupfd所对应的通道，该通道会纳入到poller来管理
            boost::any context_;    //

            // scratch variables
            ChannelList activeChannels_;    // 活跃通道列表 Poller返回的活动通道，vector<channel*>类型
            Channel* currentActiveChannel_; // 当前活跃通道列表

            mutable MutexLock mutex_;
            // 本线程或其它线程使用queueInLoop添加的任务，可能是I/O计算任务
            std::vector<Functor> pendingFunctors_ GUARDED_BY(mutex_);   // 队列中待处理的方法

        };
    }   // namespace net

}   // namespace muduo


#endif //MYMUDUO_EVENTLOOP_H
