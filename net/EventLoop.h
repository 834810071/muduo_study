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

        private:
            void abortNotInLoopThread();
            void handleRead();  // waked up
            void doPendingFunctors();

            void printActiveChannels() const;   // DEBUG

            typedef std::vector<Channel*> channelList;

            bool looping_;  // atomic
            std::atomic<bool> quit_;
            bool eventHandling_; // atomic
            bool callingPendingFunctors_;   // atomic
            int64_t iteration_;
            const pid_t threadId_;
            Timestamp pollReturnTime;
            std::unique_ptr<Poller> poller_;
            std::unique_ptr<TimerQueue> timerQueue_;
            int wakeupFd_;

            // 与内部类TimerQueue不同，不向客户端公开通道。
            std::

        };
    }   // namespace net

}   // namespace muduo


#endif //MYMUDUO_EVENTLOOP_H
