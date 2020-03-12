//
// Created by jxq on 19-8-29.
//

#include "TcpServer.h"
#include <boost/bind.hpp>
#include "../base/Logging.h"
#include "SocketsOps.h"

using namespace muduo;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr, const string& nameArg, Option option)
    : loop_(loop),
      ipPort_(listenAddr.toIpPort()),
      // name_(listenAddr.toHostPort()),   // 地址：端口号\
      name_(nameArg),
      acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop_)),
      started_(false),
      nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
            boost::bind(&TcpServer::newConnection, this, _1, _2)
            );
}

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr)
        : loop_(loop),
          ipPort_(listenAddr.toIpPort()),
          name_(listenAddr.toHostPort()),   // 地址：端口号
          acceptor_(new Acceptor(loop, listenAddr)),
          threadPool_(new EventLoopThreadPool(loop_)),
          started_(false),
          nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
            boost::bind(&TcpServer::newConnection, this, _1, _2)
    );
}

TcpServer::~TcpServer()
{
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

    for (auto& item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
                std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::start()
{
    if (!started_)
    {
        started_ = true;
        threadPool_->start();
    }

    if (!acceptor_->listenning())
    {
        loop_->runInLoop(
                boost::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr)
{
    loop_->assertInLoopThread();
    char buf[32];
    //snprintf(buf, sizeof buf, "#%d", nextConnId_);
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_
             << "] - new connection [" << connName
             << "] from " << peerAddr.toHostPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    EventLoop* ioLoop = threadPool_->getNextLoop();         // 取得EventLoop.
    // FIXME poll with zero timeout to double confirm the new connectio
    TcpConnectionPtr conn(new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));    // 创建TcpConnection对象conn
    connections_[connName] = conn;  // 加入ConnectionMap
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
            boost::bind(&TcpServer::removeConnection, this, _1));   // 向TcpConnection注册CloseCallback，用于接收连接断开的消息。
    ioLoop->runInLoop(boost::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
    loop_->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
             << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());    // 解除TcpServer对connection的使用, 引用计数降为1 ????
    assert(n == 1); (void)n;
    EventLoop* ioloop = conn->getLoop();
    ioloop->queueInLoop(boost::bind(&TcpConnection::connectDestroyed, conn));   // 回到connection自己的loop中销毁连接connectDestroyed()
}

void TcpServer::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}