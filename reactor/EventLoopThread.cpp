//
// Created by jxq on 19-8-27.
//

#include <boost/bind.hpp>
#include "EventLoopThread.h"

using namespace muduo;

EventLoopThread::EventLoopThread()
    : loop_(NULL),
    exiting_(false),
    thread_(boost::bind(&EventLoopThread::threadFunc, this)),   // 注册回调函数
    mutex_(),
    cond_(mutex_)
{

}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    loop_->quit();
    thread_.join();
}

/*
 * 线程池所在线程在每创建一个EventLoopThread后会调用相应对象的startLoop函数，注意主线程和子线程之分
 * 主线程是TcpServer所在线程，也是线程池所在线程
 * 子线程是由线程池通过pthread_create创建的线程，每一个子线程运行一个EventLoop::loop
 *
 * 1.主线程EventLoopThreadPool创建EventLoopThread对象
 *
 * 2.主线程EventLoopThread构造函数中初始化线程类Thread并传递回调函数EventLoopThread::threadFunc
 *
 * 3.主线程EventLoopThreadPool创建完EventLoopThread后，调用EventLoopThread::startLoop函数
 *
 * 4.主线程EventLoopThread::startLoop函数开启线程类Thread，即调用Thread::start
 *
 * 5.主线程Thread::start函数中使用pthread_create创建线程后
 *   子线程调用回调函数EventLoopThread::threadFunc，主线程返回到EventLoopThread::startLoop
 *
 * 6.主线程EventLoopThread::startLoop由于当前事件驱动循环loop_为null（构造时初始化为null）导致wait
 *
 * 7.子线程EventLoopThread::threadFunc创建EventLoop并赋值给loop_，然后唤醒阻塞在cond上的主线程
 *
 * 8.主线程EventLoopThread::startLoop被唤醒后，返回loop_给EventLoopThreadPool
 *
 * 9.主线程EventLoopThreadPool保存返回的loop_，存放在成员变量std::vector<EventLoop*> loops_中
 *
 * 10.子线程仍然在threadFunc中，调用EventLoop::loop函数，无限循环监听
 */
EventLoop* EventLoopThread::startLoop()
{
    assert(!thread_.started());
    thread_.start();

    {
        MutexLockGuard lock(mutex_);
        while (loop_ == NULL)
        {
            cond_.wait();
        }
    }
    return loop_;
}

// 绑定到 thread_ 中
void EventLoopThread::threadFunc()
{
    EventLoop loop;

    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();
}