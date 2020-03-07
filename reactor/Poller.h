//
// Created by jxq on 19-8-22.
//

#ifndef MYMUDUO_POLLER_H
#define MYMUDUO_POLLER_H

#include "EventLoop.h"
#include "../base/Timestamp.h"
#include <boost/core/noncopyable.hpp>
#include <vector>
#include <map>

struct pollfd;

namespace muduo
{

class Channel;

///
/// IO Multiplexing with poll(2).
///
/// This class doesn't own the Channel objects.
class Poller : boost::noncopyable
{
public:
    typedef vector<Channel*> ChannelList;

    Poller(EventLoop* loop);
    ~Poller();

    /// Polls the I/O events;
    /// Must be called in the loop thread
    Timestamp poll(int timeoutMs, ChannelList* activeChannels);

    /// Changes the interested I/O events
    /// Must be called in the loop thread
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

    void assertInLoopThread()
    {
        ownerLoop_->assertInLoopThread();
    }

private:
    void fillActiveChannel(int numEvents,
            ChannelList* activeChannels) const;

    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel*> ChannelMap;

    EventLoop* ownerLoop_;
    PollFdList pollfds_;
    ChannelMap channels_;   // fd 到 Channel 的映射
};

}

#endif //MYMUDUO_POLLER_H
