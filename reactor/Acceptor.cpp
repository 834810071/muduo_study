//
// Created by jxq on 19-8-28.
//

#include <boost/bind.hpp>
#include "Acceptor.h"
#include "SocketsOps.h"

using namespace muduo;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie()), // 创建套接字 socket
      acceptChannel_(loop_, acceptSocket_.fd()),
      listenning_(false)
{
    acceptSocket_.setReuseAddr(true);   // 重复使用
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAdddr(listenAddr);    // 绑定套接字    bind
    acceptChannel_.setReadCallback(boost::bind(&Acceptor::handleRead, this));
}

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
        : loop_(loop),
          acceptSocket_(sockets::createNonblockingOrDie()), // 创建套接字 socket
          acceptChannel_(loop_, acceptSocket_.fd()),
          listenning_(false)
{
    acceptSocket_.setReuseAddr(true);   // 重复使用
    acceptSocket_.bindAdddr(listenAddr);    // 绑定套接字    bind
    acceptChannel_.setReadCallback(boost::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen(); // 监听 listen
    acceptChannel_.enableReading(); // 向poller注册
}

void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    InetAddress peerAddr(0);
    int connfd = acceptSocket_.accept(&peerAddr);   // 允许连接 accept
    if (connfd >= 0)
    {
        if (newConnectionCallback_)
        {
            newConnectionCallback_(connfd, peerAddr);
        } else
        {
            sockets::close(connfd);
        }
    }
}











