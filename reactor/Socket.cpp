//
// Created by jxq on 19-8-28.
//

#include <strings.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "Socket.h"
#include "SocketsOps.h"
#include "../base/Logging.h"

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
    struct sockaddr_in6 addr;
    bzero(&addr, sizeof addr);
    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0)
    {
        peeraddr->setSockAddrInet6(addr);    // 设置客户端地址
    }

    return connfd;
}

void Socket::setReuseAddr(bool on)
{
    int opeval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
            &opeval, sizeof opeval);
}

void Socket::shutdownWrite()
{
    sockets::shutdownWrite(sockfd_);
}

// 禁用Nagle算法
void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
            &optval, sizeof optval);
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                           &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    }
#else
    if (on)
  {
    LOG_ERROR << "SO_REUSEPORT is not supported.";
  }
#endif
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t >(sizeof optval));
}