// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "../base/Logging.h"
#include "Channel.h"
#include "EventLoop.h"

#include <sstream>

#include <poll.h>

using namespace muduo;
using namespace muduo::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd__)
  : loop_(loop),
    fd_(fd__),
    events_(0),
    revents_(0),
    index_(-1),
    logHup_(true),
    tied_(false),
    eventHandling_(false),
    addedToLoop_(false)
{
}

Channel::~Channel()
{
  assert(!eventHandling_);
  assert(!addedToLoop_);
  if (loop_->isInLoopThread())
  {
    assert(!loop_->hasChannel(this));
  }
}

void Channel::tie(const std::shared_ptr<void>& obj)
{
  tie_ = obj;
  tied_ = true;
}

void Channel::update()  // 更新事件类型
{
  addedToLoop_ = true;
  loop_->updateChannel(this); // 调用loop的，loop再调用poll的注册pollfd
}

void Channel::remove()
{
  assert(isNoneEvent());
  addedToLoop_ = false;
  loop_->removeChannel(this);
}

// 处理所有发生的事件，如果活着，底层调用handleEventWithGuard
void Channel::handleEvent(Timestamp receiveTime)  // 事件到来调用handleEvent处理
{
  std::shared_ptr<void> guard;
  if (tied_)
  {
    guard = tie_.lock();
    if (guard)
    {
      handleEventWithGuard(receiveTime);  // 处理所有发生的事件
    }
  }
  else
  {
    handleEventWithGuard(receiveTime);
  }
}

// 处理所有发生的事件
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
  eventHandling_ = true;
  LOG_TRACE << reventsToString();
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) // 判断返回事件类型
  {
    if (logHup_)  // 如果有POLLHUP事件，输出警告信息
    {
      LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    }
    if (closeCallback_) closeCallback_(); // 调用关闭回调函数
  }

  if (revents_ & POLLNVAL)  // 不合法文件描述符，并没有终止，因为服务器程序要保证一天二十四小时工作。
  {
    LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
  }

  if (revents_ & (POLLERR | POLLNVAL))
  {
    if (errorCallback_) errorCallback_();
  }
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))  // POLLRDHUP是对端关闭连接事件，如shutdown等
  {
    if (readCallback_) readCallback_(receiveTime);
  }
  if (revents_ & POLLOUT)
  {
    if (writeCallback_) writeCallback_();
  }
  eventHandling_ = false;
}

string Channel::reventsToString() const
{
  return eventsToString(fd_, revents_);
}

string Channel::eventsToString() const
{
  return eventsToString(fd_, events_);
}

string Channel::eventsToString(int fd, int ev)
{
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & POLLIN)
    oss << "IN ";
  if (ev & POLLPRI)
    oss << "PRI ";
  if (ev & POLLOUT)
    oss << "OUT ";
  if (ev & POLLHUP)
    oss << "HUP ";
  if (ev & POLLRDHUP)
    oss << "RDHUP ";
  if (ev & POLLERR)
    oss << "ERR ";
  if (ev & POLLNVAL)
    oss << "NVAL ";

  return oss.str();
}
