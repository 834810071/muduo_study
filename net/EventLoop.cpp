//
// Created by jxq on 19-7-10.
//

#include "EventLoop.h"

#include "../base/Logging.h"
#include "../base/Mutex.h"
#include "Channel.h"
#include "Poller.h"
#include "TimerQueue.h"
#include "SocketsOps.h"

#include <algorithm>
#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
    __thread EventLoop* t_loopInThisThread = 0;

    const int kPollTimesMs = 10000; // Poll默认时间

    // Description : eventfd是一种线程间通信机制。简单来说eventfd就是一个
    // 文件描述符，它引用了一个内核维护的eventfd object，是uint64_t类型，
    // 也就是8个字节，可以作为counter。支持read，write，以及有关epoll等操作。
    // 创建事件fd
    int createEventfd()
    {
        // Return file descriptor for generic event channel.  Set initial
        //   value to COUNT.
        int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);   // 主进程/子进程信息传递

        if (evtfd < 0)
        {
            LOG_SYSERR << "Failed in eventfd";
            abort();
        }
        return evtfd;
    }


// Description : SIGPIPE的默认行为是终止进程，在命令行程序中这是合理的，
// 但是在网络编程中，这以为这如果对方断开连接而本地继续写入的话，会造成
// 服务进程意外退出。假如服务进程繁忙，没有及时处理对方断开连接的事件，
// 就有可能出现在连接断开之后继续发送数据的情况。这样的情况是需要避免的。
#pragma GCC diagnostic ignored "-Wold-style-cast"
// 屏蔽SigPipe信号，防止中断输出
class IgnoreSigPipe
{
    public:
        IgnoreSigPipe()
        {
            ::signal(SIGPIPE, SIG_IGN);
        }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;


}   // namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

// Description : 创建轮询器（Poller），创建用于传递消息的管道，初始化
// 各个部分，然后进入一个无限循环，每一次循环中调用轮询器的轮询函数
//（等待函数），等待事件发生，如果有事件发生，就依次调用套接字
//（或其他的文件描述符）的事件处理器处理各个事件，然后调用投递的
// 回调函数；接着进入下一次循环。

//EventLoop初始化时
//a、looping、quit_、eventHanding_、callingPendingFunctors_、iteration_默认初始化false
//b、设置当前线程ID。threadId_
//c、创建一个poll
//d、创建一个定时器队列
//e、创建一个唤醒事件fd
//f、创建一个唤醒事件通道
EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      iteration_(0),
      threadId_(CurrentThread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),   // 创建eventfd作为线程间等待/通知机制
      wakeupChannel_(new Channel(this, wakeupFd_)), // 创建wakeupChannel通道
      currentActiveChannel_(NULL)
{
    LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;
    if (t_loopInThisThread)
    {
        LOG_FATAL << "Anthor EventLoop " << t_loopInThisThread
                  << " exists in this thread " << threadId_;
    } else
    {
        // this赋给线程局部数据指针，凭借这个这以保证per thread a EventLoop
        t_loopInThisThread = this;
    }
    // 设置唤醒事件处理器的读回调函数为handleRead
    wakeupChannel_->setReadCallback(
            std::bind(&EventLoop::handleRead, this)
            );
    // 启用读功能(注册)
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
    LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
              << " destructs in thread " << CurrentThread::tid();
    wakeupChannel_->disableAll();   // 滞空
    wakeupChannel_->remove();       // 移除
    ::close(wakeupFd_);             // Close the file descriptor FD.
    t_loopInThisThread = NULL;
}

// Description : 事件循环。在某线程中实例化EventLoop对象，这个线程
// 就是IO线程，必须在IO线程中执行loop()操作，在当前IO线程中进行
// updateChannel，在当前线程中进行removeChannel。
void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();   // 判断在当前线程
    looping_ = true;
    quit_ = false;  // FIXME: what if someone calls quit() before loop()?
    LOG_TRACE << "EventLoop " << this << " start looping";

    while (!quit_)
    {
        activeChannels_.clear();
        // 调用poll获得活跃的channel（activeChannels_） 定期查询是否有活跃的通道
        pollReturnTime_ = poller_->poll(kPollTimesMs, &activeChannels_);
        // 增加Poll次数
        ++iteration_;
        if (Logger::logLevel() <= Logger::TRACE)
        {
            printActiveChannels();  // 日志登记，日志打印
        }
        // TODO sort channel by priority
        // 事件处理开始
        eventHandling_ = true;
        // 遍历活跃的channel，执行每一个channel上的回调函数
        for (Channel* channel : activeChannels_)
        {
            currentActiveChannel_ = channel;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }
        currentActiveChannel_ = NULL;
        eventHandling_ = false;
        // 处理用户在其他线程注册给IO线程的事件 处理下队列中是否有需要执行的回调方法
        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::quit()
{
    //loop退出
    //a、状态置位
    //b、如果在当前线程，就唤醒下fd
    quit_ = true;
    // There is a chance that loop() just executes while(!quit_) and exits,
    // then EventLoop destructs, then we are accessing an invalid object.
    // Can be fixed using mutex_ in both places.
    if (!isInLoopThread())  // 如果不在当前线程
    {
        wakeup();
    }
}

// Description : EventLoop线程除了等待poll、执行poll返回的激活事件，
// 还可以处理一些其他任务，例如调用某一个回调函数，处理其他EventLoop
// 对象的，调用EventLoop::runInLoop(cb)即可让EventLoop线程执行cb函数。
//
// 假设我们有这样的调用：loop->runInLoop(run)，说明想让IO线程执行一定
// 的计算任务，此时若是在当前的IO线程，就马上执行run()；如果是其他线程
// 调用的，那么就执行queueInLoop(run),将run异步添加到队列，当loop内处理
// 完事件后，就执行doPendingFunctors()，也就执行到了run()；最后想要结束
// 线程的话，执行quit。
void EventLoop::runInLoop(muduo::net::EventLoop::Functor cb)
{
    //在EventLoop中执行回调函数
    //a、如果Loop在自身当前线程，直接执行
    //b、如果Loop在别的线程，则将回调放到Loop的
    //   方法队列，保证回调在Loop所在的线程中执行

    // 如果当前线程是EventLoop线程则立即执行，否则放到任务队列中，异步执行
    if (isInLoopThread())
    {
        cb();
    } else
    {
        queueInLoop(std::move(cb));
    }
}

// 将任务添加到队列当中，队就是成员pendingFunctors_数组容器
void EventLoop::queueInLoop(muduo::net::EventLoop::Functor cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }

    // 如果当前线程不是EventLoop线程，或者正在执行pendingFunctors_中的任务，
    // 都要唤醒EventLoop线程，让其执行pendingFunctors_中的任务
    if (!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}

size_t EventLoop::queueSize() const
{
    MutexLockGuard lock(mutex_);
    return pendingFunctors_.size();
}

// Description : 以下三个函数是设置定时器的回调函数。
// 在指定的时间调用TimerCallback
TimerId EventLoop::runAt(muduo::Timestamp time, muduo::net::TimerCallback cb)
{
    return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

// 等一段时间之后调用TimerCallback
TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::now(), delay));
    return runAt(time, std::move(cb));
}

// 以固定的时间反复调用TimerCallback
TimerId EventLoop::runEvery(double interval, muduo::net::TimerCallback cb)
{
    Timestamp time(addTime(Timestamp::now(), interval));
    return timerQueue_->addTimer(std::move(cb), time, interval);
}

// 取消Timer 取消某个定时器事件
void EventLoop::cancel(muduo::net::TimerId timerId)
{
    return timerQueue_->cancel(timerId);
}

// 更新Channel，实际上是调用poller_->updateChannel()。
void EventLoop::updateChannel(muduo::net::Channel *channel)
{
    // Channel所在的Loop要和当前Loop保持一致
    assert(channel->ownerLoop() == this);
    // Loop所在线程不一致时，输出日志
    assertInLoopThread();
    // 更新通道事件
    poller_->updateChannel(channel);
}

// EventLoop对象中的poller对象也持有Channel对象的指针，
// 所以需要将channel对象安全的从poller对象中移除。
void EventLoop::removeChannel(muduo::net::Channel *channel)
{
    // 每次间接的调用的作用就是将需要改动的东西与当前调用的类撇清关系
    assert(channel->ownerLoop() == this);
    // 如果没有在loop线程调用直接退出
    assertInLoopThread();
    // 判断是否在事件处理状态。判断当前是否在处理这个将要删除的事件以及活动的事件表中是否有这个事件
    if (eventHandling_)
    {
        assert((currentActiveChannel_ = channel) || (std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end()));
    }
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(muduo::net::Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
              << " was created in threadId_ = " << threadId_
              << ", current thread id = " << CurrentThread::tid();
}

//这里用到的一个设计就是添加了
//回调函数到队列，通过往fd写个
//标志来通知，让阻塞的Poll立马
//返回去执行回调函数。

// 使用eventfd唤醒，写一个8字节数据
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);    // 随便写点数据进去就唤醒
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
     }
}

// 读下8字节数据
void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}

// 执行任务队列中的任务
// 1. 不是简单的在临界区内依次调用functor，而是把回调列表swap到functors中，这一方面减小了
//    临界区的长度，意味着不会阻塞其他线程的queueInLoop()，另一方面也避免了死锁(因为Functor可能再次调用quueInLoop)
// 2. 由于doPendingFunctors()调用的Functor可能再次调用queueInLoop(cb)，这是queueInLoop()就必须wakeup(),否则新增的cb可能就不能及时调用了
// 3. muduo没有反复执行doPendingFunctors()直到pendingFunctors为空，这是有意的，否则I/O线程可能陷入死循环，无法处理I/O事件
void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    // 交换出来，避免调用时长时间占用锁
    // 注意这里的临界区，这里使用了一个栈上变量functors和pendingFunctors交换
    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);    // 把它和空vector交换
    }

    //此处其它线程就可以往pendingFunctors添加任务

    //调用回调任务
    //这一部分不用临界区保护
    for (const Functor& functor : functors)
    {
        functor();
    }
    callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
    for (const Channel* channel : activeChannels_)
    {
        LOG_TRACE << "{" << channel->reventsToString() << "} ";
    }
}





















