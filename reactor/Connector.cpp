//
// Created by jxq on 19-9-3.
//

#include <boost/bind.hpp>
#include "Connector.h"
#include "../base/Logging.h"
#include "Channel.h"
#include "SocketsOps.h"

using namespace muduo;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(kDisconnected),
      retryDelayMs_(kInitRetryDelayMs)
{
    LOG_DEBUG << "ctor[ " << this << " ]";
}

Connector::~Connector()
{
    LOG_DEBUG << "dtor[" << this << "]";
    loop_->cancel(timerId_);
    assert(!channel_);
}

void Connector::start()
{
    connect_ = true;
    loop_->runInLoop(boost::bind(&Connector::startInLoop, this));   // FIXME : unsafe
}

void Connector::restart()
{
    loop_->assertInLoopThread();
    setState(kDisconnected);
    retryDelayMs_ = kInitRetryDelayMs;
    connect_ = true;
    startInLoop();
}

void Connector::stop()
{
    connect_ = false;
    loop_->cancel(timerId_);
}

void Connector::startInLoop()
{
    loop_->assertInLoopThread();
    assert(state_ == kDisconnected);
    if (connect_)
    {
        connect();
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::connect(){
    int sockfd = sockets::createNonblockingOrDie(); // socket
    int ret = sockets::connect(sockfd, serverAddr_.getSockAddrInet());  // connect
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(sockfd);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
            sockets::close(sockfd);
            break;

        default:
            LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
            sockets::close(sockfd);
            // connectErrorCallback_();
            break;
    }
}

void Connector::connecting(int sockfd){
    setState(kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->setWriteCallback(
            boost::bind(&Connector::handleWrite, this));    // FIXME : unsafe

    channel_->setErrorCallback(
            boost::bind(&Connector::handleError, this)); // FIXME: unsafe

    // channel_->tie(shared_from_this()); is not working,
    // as channel_ is not managed by shared_ptr
    channel_->enableWriting();
}

void Connector::handleWrite(){
    LOG_TRACE << "Connector::handleWrite " << state_;

    if (state_ == kConnecting)
    {
        int sockfd = removeAndResetChannel();
        int err = sockets::getSocketError(sockfd);
        if (err)
        {
            LOG_WARN << "Connector::handleWrite - SO_ERROR = "
                << err << " " << strerror_tl(err);
            retry(sockfd);
        }
        else if (sockets::isSelfConnect(sockfd))
        {
            LOG_WARN << "Connector::handleWrite - Self connect";
            retry(sockfd);
        }
        else
        {
            setState(kConnected);
            if (connect_)
            {
                newConnectionCallback_(sockfd);
            }
            else
            {
                sockets::close(sockfd);
            }
        }
    }
    else
    {
        // what happened?
        assert(state_ == kDisconnected);
    }
}

void Connector::handleError(){
    LOG_ERROR << "Connector::handleError";
    assert(state_ == kConnecting);

    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
    retry(sockfd);
}

// 关闭重连
void Connector::retry(int sockfd){
    sockets::close(sockfd);          // close
    setState(kDisconnected);
    if (connect_)
    {
        LOG_INFO << "Connector::retry - Retry connecting to "
                 << serverAddr_.toHostPort() << " in "
                 << retryDelayMs_ << " milliseconds. ";
        timerId_ =
        loop_->runAfter(retryDelayMs_/1000.0,  // FIXME: unsafe
                                   boost::bind(&Connector::startInLoop, this));
        retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}

// 连接成功的话 那么需要清空该sockfd以及相关的东西
int Connector::removeAndResetChannel(){
    channel_->disableAll();
    loop_->removeChannel(get_pointer(channel_));
    int sockfd = channel_->fd();
    // Can't reset channel_ here, because we are inside Channel::handleEvent
    loop_->queueInLoop(boost::bind(&Connector::resetChannel, this)); // FIXME: unsafe
    return sockfd;
}

void Connector::resetChannel(){
    channel_.reset();
}