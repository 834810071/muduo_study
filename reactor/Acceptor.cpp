//
// Created by jxq on 19-8-28.
//

#include <boost/bind.hpp>
#include "Acceptor.h"
#include "SocketsOps.h"

using namespace muduo;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie()), // 创建套接字
      acceptChannel_(loop_, acceptSocket_.fd()),
      listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAdddr(listenAddr);    // 绑定套接字
    acceptChannel_.setReadCallback(boost::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen(); // 监听
    acceptChannel_.enableReading();
}

void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    InetAddress peerAddr(0);
    int connfd = acceptSocket_.accept(&peerAddr);   // 允许连接
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











