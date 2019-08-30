//
// Created by jxq on 19-8-29.
//

#ifndef MYMUDUO_TCPSERVER_H
#define MYMUDUO_TCPSERVER_H


#include <boost/core/noncopyable.hpp>
#include <map>
#include "../s04/InetAddress.h"
#include "TcpConnection.h"
#include "../s04/Acceptor.h"
#include "../s00/EventLoop.h"
//#include "Callbacks.h"

namespace muduo
{

class Acceptor;
class EventLoop;

class TcpServer : boost::noncopyable {
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr);
    ~TcpServer();   // force out-line dtor, for scoped_ptr members.

    /// Starts the server if it's not listenning.
    ///
    /// It's harmless to call it multiple times.
    /// Thread safe.
    void start();

    /// Set connection callback.
    /// Not thread safe.
    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

private:
    // Not thread safe, but in loop
    void newConnection(int sockfd, const InetAddress& peerAddr);

    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop* loop_;   // the acceptor loop
    const std::string name_;
    boost::scoped_ptr<Acceptor> acceptor_;  // avoid revealing Acceptor
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    bool started_;
    int nextConnId_;    // always in loop thread
    ConnectionMap connections_;
};

}


#endif //MYMUDUO_TCPSERVER_H
