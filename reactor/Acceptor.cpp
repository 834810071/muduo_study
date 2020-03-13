//
// Created by jxq on 19-8-28.
//

#include <boost/bind.hpp>
#include <fcntl.h>
#include "Acceptor.h"
#include "SocketsOps.h"
#include "../base/Logging.h"

using namespace muduo;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie()), // 创建套接字 socket
      acceptChannel_(loop_, acceptSocket_.fd()),
      listenning_(false),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
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
    else
    {
        LOG_SYSERR << "in Acceptor::handleRead";
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.
        if (errno == EMFILE)    // 当文件描述符达到上限
        {
            ::close(idleFd_);   //  先关闭空闲文件
            idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL); // accept拿到新的连接
            ::close(idleFd_);   // 再断开连接
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC); // 重新打开空闲
        }
    }
}











