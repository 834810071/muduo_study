//
// Created by jxq on 19-8-28.
//

#include <strings.h>
#include "Socket.h"
#include "SocketsOps.h"

using namespace muduo;

Socket::~Socket()
{
    sockets::close(sockfd_);
}

void Socket::bindAdddr(const InetAddress& addr)
{
    sockets::bindOrDie(sockfd_, addr.getSockAddrInet());
}

void Socket::listen()
{
    sockets::listenOrDie(sockfd_);
}


int Socket::accept(InetAddress* peeraddr)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0)
    {
        peeraddr->setSockAddrInet(addr);    // 设置客户端地址
    }

    return connfd;
}

void Socket::setReuseAddr(bool on)
{
    int opeval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
            &opeval, sizeof opeval);
}