//
// Created by jxq on 19-8-29.
//

#include "TcpConnection.h"
#include "../base/Logging.h"
#include <boost/bind.hpp>
#include "Callbacks.h"
#include "SocketsOps.h"

using namespace muduo;

TcpConnection::TcpConnection(EventLoop* loop,
                            const std::string& nameArg,
                            int sockfd,
                            const InetAddress& localAddr,
                            const InetAddress& peerAddr)
    : loop_(CHECK_NOTNULL(loop)),
      name_(nameArg),
      state_(kConnecting),
      socket_(new Socket(sockfd)),  // connfd
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      highWaterMark_(64*1024*1024)
{
    channel_->setReadCallback(boost::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallback(
            boost::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
            boost::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
            boost::bind(&TcpConnection::handleError, this));
    LOG_DEBUG << "TcpConnection::ctor[" <<  name_ << "] at " << this
              << " fd=" << sockfd;
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
              << " fd=" << channel_->fd();
}


/*
 * 1.创建服务器(TcpServer)时，创建Acceptor，设置接收到客户端请求后执行的回调函数
 * 2.Acceptor创建监听套接字，将监听套接字绑定到一个Channel中，设置可读回调函数为Acceptor的handleRead
 * 3.服务器启动，调用Acceptor的listen函数创建监听套接字，同时将Channel添加到Poller中
 * 4.有客户端请求连接，监听套接字可读，Channel被激活，调用可读回调函数(handleRead)
 * 5.回调函数接收客户端请求，获得客户端套接字和地址，调用TcpServer提供的回调函数(newConnection)
 * 6.TcpServer的回调函数中创建TcpConnection代表这个tcp连接，设置tcp连接各种回调函数(由用户提供给TcpServer)
 * 7.TcpServer让tcp连接所属线程调用TcpConnection的connectEstablished
 * 8.connectEstablished开启对客户端套接字的Channel的可读监听，然后调用用户提供的回调函数
 */
void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    //char buf[65536];
    int savedErrno = 0;
   // ssize_t n = ::read(socket_->fd(), buf, sizeof buf);
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0)
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0)
    {
        handleClose();
    }
    else
    {
        errno = savedErrno;
        LOG_SYSERR << "TcpConnection:handleRead";
        handleError();
    }

}

void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if (channel_->isWriting())
    {
        ssize_t n = ::write(channel_->fd(),
                            outputBuffer_.peek(),
                            outputBuffer_.readableBytes());
        if (n > 0)
        {
            outputBuffer_.retrieveAll();
            if (outputBuffer_.readableBytes() == 0)
            {
                channel_->disableWriting(); // 停止观察writable事件
                if (writeCompleteCallback_)
                {
                    loop_->queueInLoop(boost::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            } else
            {
                LOG_TRACE << "I am going to write more data";
            }
        }
        else
        {
            LOG_SYSERR << "TcpConnection::handleWrite";
        }
    }
    else
    {
        LOG_TRACE << "Connection is down, no more writing";
    }
}

void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << state_;
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor[析构函数], so we can find leaks easily.
    channel_->disableAll();
    // must be the last line
    closeCallback_(shared_from_this()); // 回调绑定到 TcpServer::handleClose()
}

void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError [" << name_
              << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisconnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());

    loop_->removeChannel(get_pointer(channel_));
}


void TcpConnection::shutdown()
{
    // FIXME: use compare and swap
    if (state_ == kConnected)
    {
        setState(kDisconnecting);
        // FIXME: shared_from_this()?
        loop_->runInLoop(boost::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::setTcpNoDelay(bool on)
{
   socket_->setTcpNoDelay(on);
}

void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if (!channel_->isWriting())
    {
        // we are not writing
        socket_->shutdownWrite();
    }
}

void TcpConnection::send(const std::string& message)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(message);
        }
        else
        {
            loop_->runInLoop(boost::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

// FIXME efficiency!!!
void TcpConnection::send(Buffer* buf)
{
    if (state_ == kConnected)
    {
        if (loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            void (TcpConnection::*fp)(const string& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(
                    std::bind(fp,
                              this,     // FIXME
                              buf->retrieveAllAsString()));
            //std::forward<string>(message)));
        }
    }
}

void TcpConnection::sendInLoop(const std::string& message)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = ::write(channel_->fd(), message.data(), message.size());
        if (nwrote >= 0)
        {
            if (implicit_cast<size_t >(nwrote) < message.size())
            {
                LOG_TRACE << "I am going to write more data";
            }
            else if (writeCompleteCallback_)
            {
                loop_->queueInLoop(
                        boost::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)   // 用于非阻塞模式，不需要重新读或者写
            {
                LOG_SYSERR << "TcpConnection::sendInLoop";
            }
        }
    }

    assert(nwrote >= 0);
    if (implicit_cast<size_t >(nwrote) < message.size())
    {
        outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::sendInLoop(const void* data, size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected)
    {
        LOG_WARN << "disconnected, give up writing";
        return;
    }
    // if no thing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = sockets::write(channel_->fd(), data, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else // nwrote < 0
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_SYSERR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others?
                {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0)
    {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_)
        {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
        if (!channel_->isWriting())
        {
            channel_->enableWriting();
        }
    }
}














