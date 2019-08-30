//
// Created by jxq on 19-8-22.
//

#ifndef MYMUDUO_CHANNEL_H
#define MYMUDUO_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

namespace muduo
{

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd
class Channel : boost::noncopyable {
public:
    typedef boost::function<void()> EventCallback;

    Channel(EventLoop* loop, int fd);

    ~Channel();

    void handleEvent();
    void setReadCallback(const EventCallback& cb)
    {
        readCallback_ = cb;
    }

    void setWriteCallback(const EventCallback& cb)
    {
        writeCallback_ = cb;
    }

    void setErrorCallback(const EventCallback& cb)
    {
        errorCallback_ = cb;
    }

    int fd() const
    {
        return fd_;
    }
    int events() const
    {
        return events_;
    }
    int set_revents(int revt)
    {
        revents_ = revt;
    }
    bool isNoneEvent() const
    {
        return events_ == kNoneEvent;
    }

    void enableReading()
    {
        events_ |= kReadEvent;
        update();
    }
    void enableWriting()
    {
        events_ |= kWriteEvent;
        update();
    }
    void disableWriting()
    {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll()
    {
        events_ = kNoneEvent;
        update();
    }

    // for Poller
    int index()
    {
        return index_;
    }
    void set_index(int idx)
    {
        index_ = idx;
    }

    EventLoop* ownerLoop()
    {
        return loop_;
    }


private:
    void update();
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;    // 关心的IO事件，由用户设置    // 名字来自poll(2)的struct pollfd; /* requested events */
    int revents_;   // poller返回的就绪的事件，由EventLoop/Poller设置。 返回事件   /* returned events */
    int index_;     // used by Poller  在Poller::pollfds_中的索引

    bool eventHandling_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;
};

}



#endif //MYMUDUO_CHANNEL_H
