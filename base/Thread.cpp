//
// Created by jxq on 19-6-21.
//

#include "Thread.h"
#include "CurrentThread.h"
#include "Exception.h"
#include "Timestamp.h"
//#include "Logging.h"

#include <type_traits>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <pthread.h>

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
                *tid_ = muduo::CurrentThread::tid();
                tid_ = NULL;
                latch_->countDown();    // 倒计时
                latch_ = NULL;

                muduo::CurrentThread::t_threadName = name_.empty() ? "muduoThread" : name_.c_str();
                ::prctl(PR_SET_NAME, muduo::CurrentThread::t_threadName);   // 设置进程名
                try
                {
                    func_();
                    muduo::CurrentThread::t_threadName = "finished";
                }
                catch (const Exception& ex)
                {
                    muduo::CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                    fprintf(stderr, "reason: %s\n", ex.what());
                    fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
                    abort();    // 直接直接终止程序
                }
                catch (const std::exception& ex)
                {
                    muduo::CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
                    fprintf(stderr, "reason: %s\n", ex.what());
                    abort();
                }
                catch (...) // 表示捕获所有类型的异常。
                {
                    muduo::CurrentThread::t_threadName = "crashed";
                    fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
                    throw;
                }
            }   // void runInThread()
        };  // struct ThreadData

        void* startThread(void* obj)
        {
            ThreadData* data = static_cast<ThreadData*>(obj);
            data->runInThread();
            delete(data);
            return NULL;
        }
    }   // namespace detail

    void CurrentThread::cacheTid()
    {
        if (t_cachedTid == 0)
        {
            t_cachedTid = detail::gettid();
            t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d", t_cachedTid);
        }
    }

    bool CurrentThread::isMainThread()
    {
        return tid() == ::getpid(); // returns the thread ID of the current process.
    }

    void CurrentThread::sleepUsec(int64_t usec)
    {
        struct timespec ts = {0, 0};
        ts.tv_sec = static_cast<time_t> (usec / Timestamp::kMicroSecondsPerSecond);  // 1000 * 1000
        ts.tv_nsec = static_cast<long> (usec % Timestamp::kMicroSecondsPerSecond);
        ::nanosleep(&ts, NULL); // 使进程睡眠一段时间后被唤醒
    }

    AtomicInt32 Thread::numCreated_;    // 静态变量

    Thread::Thread(ThreadFunc func, const string& n)
        : started_(false),
          joined_(false),
          pthreadId_(0),
          tid_(0),
          func_(std::move(func)),
          name_(n),
          latch_(1)
    {
        setDefaultName();
    }

    Thread::~Thread()
    {
        if (started_ && !joined_)
        {
            //  pthread_detach()即主线程与子线程分离，两者相互不干涉，子线程结束同时子线程的资源自动回收。
            pthread_detach(pthreadId_);
        }
    }

    void Thread::setDefaultName()
    {
        int num = numCreated_.incrementAndGet();
        if (name_.empty())
        {
            char buf[32];
            snprintf(buf, sizeof(buf), "Thread%d", num);
            name_ = buf;
        }
    }

    void Thread::start()
    {
        assert(!started_);
        started_ = true;
        // FIXME : move(func_)
        detail::ThreadData* data = new detail::ThreadData(func_, name_, &tid_, &latch_);
                        // 指向线程标识符的指针 设置线程属性 线程运行函数的起始地址 参数
        // pthread_create() 在调用成功完成之后返回零。其他任何返回值都表示出现了错误。
        if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))  // 创建线程
        {
            started_ = false;
            delete data;
            // LOG_SYSFATAL << "Failed in pthread_create";
        }
        else
        {
            latch_.wait();
            assert(tid_ > 0);
        }
    }

    int Thread::join()
    {
        assert(started_);
        assert(!joined_);
        joined_ = true;
        // pthread_join()即是子线程合入主线程，主线程会一直阻塞，直到子线程执行结束，然后回收子线程资源，并继续执行。
        return pthread_join(pthreadId_, NULL);
    }

}   // namespace muduo


























