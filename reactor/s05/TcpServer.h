//
// Created by jxq on 19-8-29.
//

#ifndef MYMUDUO_TCPSERVER_H
#define MYMUDUO_TCPSERVER_H


#include <boost/core/noncopyable.hpp>
#include <map>
#include "../s04/InetAddress.h"
#include "TcpConnection.h"

namespace muduo
{

class Acceptor;
class EventLoop;

class TcpServer : boost::noncopyable {
public:

private:
    // Not thread safe, but in loop
    void newConnection(int sockfd, const InetAddress& peerAddr);

    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
};

}


#endif //MYMUDUO_TCPSERVER_H
