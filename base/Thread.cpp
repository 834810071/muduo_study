//
// Created by jxq on 19-6-21.
//

#include "Thread.h"
#include "CurrentThread.h"
#include "Exception.h"
//#include "Logging.h"

#include <type_traits>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

using namespace std;

namespace muduo
{
    namespace detail
    {
        // tid 真实的线程id唯一标识
        pid_t gettid()
        {
            return static_cast<pid_t>(::syscall(SYS_gettid));
        }

        void afterFork()
        {
            muduo::CurrentThread::t_cachedTid = 0;
            muduo::CurrentThread::t_threadName = "main";
            CurrentThread::tid();
        }

        class ThreadNameInitializer
        {
        public:
            ThreadNameInitializer()
            {
                muduo::CurrentThread::t_threadName = "main";
                CurrentThread::tid();
                pthread_atfork(NULL, NULL, &afterFork);
            }
        };

        ThreadNameInitializer init;

        struct ThreadData
        {
            typedef muduo::Thread::ThreadFunc ThreadFunc;
            ThreadFunc func_;
            string name_;
            pid_t* tid_;
            CountDownLatch* latch_;

            ThreadData(ThreadFunc func,
                      const string& name,
                      pid_t* tid,
                      CountDownLatch* latch)
                : func_(move(func)),
                  name_(name),
                  tid_(tid),
                  latch_(latch)
            {

            }

            void runInThread()
            {
                *tid_ =
            }
                      _
        }
    };   // namespace detail
}   // namespace muduo
