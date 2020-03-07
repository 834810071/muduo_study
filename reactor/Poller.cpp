//
// Created by jxq on 19-8-22.
//

#include "Poller.h"
#include "../base/Logging.h"
#include "Channel.h"
#include <poll.h>

using namespace muduo;

Poller::Poller(EventLoop* loop)
    : ownerLoop_(loop)
{

}

Poller::~Poller()
{

}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
    Timestamp now(Timestamp::now());
    if (numEvents > 0)
    {
        LOG_TRACE << numEvents << " events happended";
        fillActiveChannel(numEvents, activeChannels);
    }
    else if (numEvents == 0)
    {
        LOG_TRACE << " nothing happended";
    }
    else
    {
        LOG_SYSERR << "Poller::poll()";
    }
    return now;
}

void Poller::fillActiveChannel(int numEvents, ChannelList* activeChannels) const
{
    for (PollFdList::const_iterator pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0; ++pfd)
    {
        if (pfd->revents > 0)
        {
            --numEvents;
            ChannelMap::const_iterator ch = channels_.find(pfd->fd);    // 找到响应文件描述符
            assert(ch != channels_.end());
            Channel* channel = ch->second;  // 根据文件描述符找到所属Channel
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents); // 设置返回事件/当前关心事件
            activeChannels->push_back(channel); // 添加活跃Channel
        }

    }
}

void Poller::updateChannel(muduo::Channel *channel)
{
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    if (channel->index() < 0)
    {
        // a new one, add to pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = channel->events();
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    }
    else
    {
        // update existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(channels_.size()));
        struct pollfd& pfd = pollfds_[idx];
        //assert(pfd.fd == channel->fd() || pfd.fd == -1);
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->isNoneEvent())
        {
            // ignore this pollfd;
            pfd.fd = -channel->fd()-1;
        }
    }
}

void Poller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd();
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    assert(channel->isNoneEvent());
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    const struct pollfd& pfd = pollfds_[idx]; (void)pfd;

    assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());  // 跟updateChannel函数对应
    size_t n = channels_.erase(channel->fd());
    assert(n == 1); (void)n;
    if (implicit_cast<size_t >(idx) == pollfds_.size()-1)
    {
        pollfds_.pop_back();
    }
    else
    {
        int channelAtEnd = pollfds_.back().fd;
        iter_swap(pollfds_.begin()+idx, pollfds_.end()-1);  // 将要删除的元素与最后一个元素交换
        if (channelAtEnd < 0)
        {
            channelAtEnd = -channelAtEnd - 1;   // 把它改为-fd-1，然后删除的时候可以再还原回来。如果改为-1就不可以了
        }
        channels_[channelAtEnd]->set_index(idx);    // 对该真实的fd更新下标
        pollfds_.pop_back();
    }
}