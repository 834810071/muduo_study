//
// Created by jxq on 19-8-28.
//

#ifndef MYMUDUO_SOCKET_H
#define MYMUDUO_SOCKET_H

#include <boost/core/noncopyable.hpp>
#include "InetAddress.h"

namespace muduo
{

class InetAddress;
///
/// Wrapper of socket file descriptor.
///
/// It closes the sockfd when desctructs.
/// It's thread safe, all operations are delagated to OS.
class Socket : boost::noncopyable {
public:
    explicit Socket(int sockfd)
        : sockfd_(sockfd)
    {

    }

    ~Socket();

    int fd() const
    {
        return sockfd_;
    }

    // abort if address in use
    void bindAdddr(const InetAddress& localaddr);
    // abort if address in use
    void listen();

    /// On success, returns a non-negative integer that is
    /// a descriptor for the accepted socket, which has been
    /// set to non-blocking and close-on-exec. *peeraddr is assigned.
    /// On error, -1 is returned, and *peeraddr is untouched.
    int accept(InetAddress* peeraddr);

    ///
    /// Enable/disable SO_REUSEADDR
    ///
    void setReuseAddr(bool on);

    void shutdownWrite();

    ///
    /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
    ///
    void setTcpNoDelay(bool on);
private:
    const int sockfd_;
};

}


#endif //MYMUDUO_SOCKET_H
