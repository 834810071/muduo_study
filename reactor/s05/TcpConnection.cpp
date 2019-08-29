//
// Created by jxq on 19-8-29.
//

#include "TcpConnection.h"
#include "../../base/Logging.h"
#include <boost/bind.hpp>

using namespace muduo;

TcpConnection::TcpConnection(EventLoop* loop,
                            const std::string& nameArg,
                            int sockfd,
                            const InetAddress& localAddr,
                            const InetAddress& peerAddr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(nameArg),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr)
{
    LOG_DEBUG << "TcpConnection::ctor[" <<  name_ << "] at " << this
              << " fd=" << sockfd;
    channel_->setReadCallback(boost::bind(&TcpConnection::handleRead, this));
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
              << " fd=" << channel_->fd();
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead()
{
    char buf[65536];
    ssize_t n = ::read(socket_->fd(), buf, sizeof buf);
    messageCallback_(shared_from_this(), buf, n);
    // FIXME: close connection if n == 0
}