//
// Created by jxq on 19-8-28.
//

#ifndef MYMUDUO_INETADDRESS_H
#define MYMUDUO_INETADDRESS_H


#include <cstdint>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include "../base/copyable.h"
#include "SocketsOps.h"

namespace muduo
{
///
/// Wrapper of sockaddr_in.
///
/// This is an POD interface class.
class InetAddress : public muduo::copyable{
public:
    /// Constructs an endpoint with given port number.
    /// Mostly used in TcpServer listening.
    explicit InetAddress(uint16_t port);

    /// Constructs an endpoint with given ip and port.
    /// @c ip should be "1.2.3.4"
    InetAddress(const std::string& ip, uint16_t port);

    /// Constructs an endpoint with given struct @c sockaddr_in
    /// Mostly used when accepting new connections
    InetAddress(const struct sockaddr_in& addr)
        : addr_(addr)
    {

    }

    explicit InetAddress(const struct sockaddr_in6& addr)
            : addr6_(addr)
    { }


    std::string toIpPort() const;

    std::string toHostPort() const;

    const struct sockaddr* getSockAddr() const { return sockets::sockaddr_cast(&addr6_); }
    void setSockAddrInet6(const struct sockaddr_in6& addr6) { addr6_ = addr6; }

    // default copy/assignment are Okay
    const struct sockaddr_in& getSockAddrInet() const
    {
        return addr_;
    }
    void setSockAddrInet(const struct sockaddr_in& addr)
    {
        addr_ = addr;
    }




private:
    //struct sockaddr_in addr_;
    union
    {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};

}


#endif //MYMUDUO_INETADDRESS_H
