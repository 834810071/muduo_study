//
// Created by jxq on 19-8-28.
//

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <fcntl.h>
#include "SocketsOps.h"
#include "../base/Types.h"
#include "../base/Logging.h"
#include <strings.h>

using namespace muduo;

namespace
{
//    typedef struct sockaddr SA;
//
//    const SA* sockaddr_cast(const struct sockaddr_in* addr)
//    {
//        //return static_cast<const SA*>(implicit_cast<const void*>(addr));
//        return static_cast<const SA*>(reinterpret_cast<const void*>(addr));
//    }
//
//    SA* sockaddr_cast(struct sockaddr_in* addr)
//    {
//        return static_cast<SA*>(reinterpret_cast<void*>(addr));
//    }

    void setNonBlockAndCloseExec(int sockfd)
    {
        // non-block
        // fcntl系统调用可以用来对已打开的文件描述符进行各种控制操作以改变已打开文件的的各种属性
        // https://www.cnblogs.com/zxc2man/p/7649240.html
        // 获取文件打开方式的标志，标志值含义与open调用一致
        int flags = ::fcntl(sockfd, F_GETFL, 0);
        flags |= O_NONBLOCK;
        // 设置文件状态标记
        int ret = ::fcntl(sockfd, F_SETFL, flags);

        // close-on-exec
        // 读取文件描述符close-on-exec标志
        flags = ::fcntl(sockfd, F_GETFD, 0);
        // FD_CLOEXEC：若设置某个文件的fd flag（即为1），则进程fork出子进程时，执行exec前要关闭这个文件描述符。
        flags |= FD_CLOEXEC;
        // 将文件描述符close-on-exec标志设置为第三个参数arg的最后一位
        // 设置文件描述符标记
        ret = ::fcntl(sockfd, F_SETFD, flags);
    }
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in6* addr)
{
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in6* addr)
{
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr)
{
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr)
{
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* sockets::sockaddr_in6_cast(const struct sockaddr* addr)
{
    return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}


int sockets::createNonblockingOrDie()
{
    // socket
#if VALGRIND
    int sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }

    setNonBlockAndCloseOnExec(sockfd);
#else
    int sockfd = socket(AF_INET,
            SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
            IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }
#endif
    return sockfd;
}

void sockets::listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0)
    {
        LOG_SYSFATAL << "sockets::listenOrDie";
    }
}

int sockets::accept(int sockfd, struct sockaddr_in6 *addr)
{
    socklen_t addrlen = sizeof *addr;
#if VALGRIND || defined (NO_ACCEPT4)
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
  setNonBlockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(sockfd, sockaddr_cast(addr),
                           &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (connfd < 0)
    {
        int savedErrno = errno;
        LOG_SYSERR << "Socket::accept";
        switch (savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno; // 暂时错误忽略
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept " << savedErrno;
                break;
        }
    }
    return connfd;
}

void sockets::close(int sockfd)
{
    if (::close(sockfd) < 0)
    {
        LOG_SYSERR << "sockets::close";
    }
}

void sockets::toHostPort(char *buf, size_t size,
        const struct sockaddr_in &addr)
{
    char host[INET_ADDRSTRLEN] = "INVALID";
    // 将数值格式转化为点分十进制的ip地址格式
    ::inet_ntop(AF_INET, &addr.sin_addr, host, sizeof(host));
    uint16_t port = sockets::networkToHost16(addr.sin_port);
    snprintf(buf, size, "%s:%u", host, port);
}

void sockets::fromHostPort(const char *ip, uint16_t port,
        struct sockaddr_in *addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);
    // 将点分十进制的ip地址转化为用于网络传输的数值格式
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        LOG_SYSERR << "sockets::fromHostPort";
    }
}

void sockets::bindOrDie(int sockfd, const struct sockaddr_in& addr)
{
    if (::bind(sockfd, sockaddr_cast(&addr), sizeof addr) < 0)
    {
        LOG_SYSFATAL << "sockets::bindOrDie";
    }
}

struct sockaddr_in6 sockets::getLocalAddr(int sockfd)
{
    struct sockaddr_in6 localaddr;
    bzero(&localaddr, sizeof(localaddr));
    socklen_t addlen = sizeof(localaddr);
    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addlen) < 0)
    {
        LOG_SYSERR << "sockets::getLocalAddr";
    }
    return localaddr;
}

int sockets::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = sizeof optval;
                                        // 获得套接字错误
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

void sockets::shutdownWrite(int sockfd)
{
    // 关闭sockfd的写功能，此选项将不允许sockfd进行写操作
    if (::shutdown(sockfd, SHUT_WR) < 0)
    {
        LOG_SYSERR << "sockets::shutdownWrite";
    }
}

int sockets::connect(int sockfd, const struct sockaddr_in& addr)
{
    return ::connect(sockfd, sockaddr_cast(&addr), sizeof addr);
}

bool sockets::isSelfConnect(int sockfd)
{
    struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
    struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
    if (localaddr.sin6_family == AF_INET)
    {
        const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peeraddr);
        return laddr4->sin_port == raddr4->sin_port
               && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    }
    else if (localaddr.sin6_family == AF_INET6)
    {
        return localaddr.sin6_port == peeraddr.sin6_port
               && memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr, sizeof localaddr.sin6_addr) == 0;
    }
    else
    {
        return false;
    }
//    struct sockaddr_in6 localaddr = getLocalAddr(sockfd);
//    struct sockaddr_in6 peeraddr = getPeerAddr(sockfd);
//    return localaddr.sin_port == peeraddr.sin_port
//           && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}

struct sockaddr_in6 sockets::getPeerAddr(int sockfd)
{
    struct sockaddr_in6 peeraddr;
    bzero(&peeraddr, sizeof peeraddr);
    socklen_t addrlen = sizeof(peeraddr);
    if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
    {
        LOG_SYSERR << "sockets::getPeerAddr";
    }
    return peeraddr;
}


void sockets::toIpPort(char* buf, size_t size,
                       const struct sockaddr* addr)
{
    toIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    uint16_t port = sockets::networkToHost16(addr4->sin_port);
    assert(size > end);
    snprintf(buf+end, size-end, ":%u", port);
}


void sockets::toIp(char* buf, size_t size,
                   const struct sockaddr* addr)
{
    if (addr->sa_family == AF_INET)
    {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    }
    else if (addr->sa_family == AF_INET6)
    {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

ssize_t sockets::write(int sockfd, const void *buf, size_t count)
{
    return ::write(sockfd, buf, count);
}











