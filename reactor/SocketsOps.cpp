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
    typedef struct sockaddr SA;

    const SA* sockaddr_cast(const struct sockaddr_in* addr)
    {
        //return static_cast<const SA*>(implicit_cast<const void*>(addr));
        return static_cast<const SA*>(reinterpret_cast<const void*>(addr));
    }

    SA* sockaddr_cast(struct sockaddr_in* addr)
    {
        return static_cast<SA*>(reinterpret_cast<void*>(addr));
    }

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
    int sockfd = ::socket(AF_INET,
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

int sockets::accept(int sockfd, struct sockaddr_in *addr)
{
    socklen_t addrlen = sizeof *addr;
#if VALGRIND
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonBlockAndCloseOnExec(sockfd);
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

struct sockaddr_in sockets::getLocalAddr(int sockfd)
{
    struct sockaddr_in localaddr;
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



















