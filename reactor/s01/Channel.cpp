//
// Created by jxq on 19-8-22.
//

#include "Channel.h"
#include "../../base/Logging.h"
#include "../s00/EventLoop.h"
#include <poll.h>

using namespace muduo;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;   // POLLPRI 有紧迫数据可读    POLLIN 有数据可读
const int Channel::kWriteEvent = POLLOUT;           // POLLOUT 写数据不会导致阻塞

Channel::Channel(EventLoop* loop, int fdArg)
    : loop_(loop),
      fd_(fdArg),
      events_(0),
      revents_(0),
      index_(-1)
{

}

void Channel::update()
{
    loop_->updateChannel(this);
}

void Channel::handleEvent()
{
    if (revents_ & POLLNVAL)    // POLLNVAL 指定的文件描述符非法
    {
        LOG_WARN << "Channel::handleEvent() POLLNVAL";
    }

    if (revents_ & (POLLERR | POLLNVAL))    // POLLERR 指定的文件描述符发生错误
    {
        if (errorCallback_)
        {
            errorCallback_();
        }
    }

    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))  // POLLRDHUP 在socket上接收到对方关闭连接的请求之后触发
    {
        if (readCallback_)
        {
            readCallback_();
        }
    }

    if (revents_ & POLLOUT)
    {
        if (writeCallback_)
        {
            writeCallback_();
        }
    }
}