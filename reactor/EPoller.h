//
// Created by jxq on 19-9-5.
//

#ifndef MYMUDUO_EPOLLER_H
#define MYMUDUO_EPOLLER_H

#include <boost/core/noncopyable.hpp>
#include <vector>
#include <map>
#include <sys/epoll.h>

#include "EventLoop.h"
#include "Channel.h"

using namespace std;

struct epoll_event;

namespace muduo
{

class Channel;

class EPoller : boost::noncopyable{
public:
    typedef std::vector<Channel*> ChannelList;

    EPoller(EventLoop* loop);
    ~EPoller();

    /// Polls the I/O events.
    /// Must be called in the loop thread.
    Timestamp poll(int timeoutMs, ChannelList* activeChannels);

    /// Changes the interested I/O events.
    /// Must be called in the loop thread.
    void updateChannel(Channel* channel);
    /// Remove the channel, when it destructs.
    /// Must be called in the loop thread.
    void removeChannel(Channel* channel);

    void assertInLoopThread() { ownerLoop_->assertInLoopThread(); }

private:
    static const int kInitEventListSize = 16;

    void fillActiveChannel(int numEvents,
                           ChannelList* activeChannels) const;
    void update(int operation, Channel* channel);

    typedef std::vector<struct epoll_event> EventList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* ownerLoop_;
    int epollfd_;       // ::epoll_create()
    EventList events_;  // 不是保存所有关注的fd列表，而是一次epoll_wait(2)调用返回的活动fd列表，它的大小是自适应的
    ChannelMap channels_;
};

}


#endif //MYMUDUO_EPOLLER_H
