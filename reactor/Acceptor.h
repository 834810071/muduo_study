//
// Created by jxq on 19-8-28.
//

#ifndef MYMUDUO_ACCEPTOR_H
#define MYMUDUO_ACCEPTOR_H


#include <boost/core/noncopyable.hpp>
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"

namespace muduo
{

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
///
class Acceptor : boost::noncopyable{
public:
    typedef boost::function<void(int sockfd,
            const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);   // socket、bind
    Acceptor(EventLoop* loop, const InetAddress& listenAddr);

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback_ = cb;
    }

    bool listenning() const
    {
        return listenning_;
    }

    void listen();  // 执行创建TCP服务端的传统步骤, listen
private:
    void handleRead();

    EventLoop* loop_;
    Socket acceptSocket_;   // RAII handle, 监听socket
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;
};

}


#endif //MYMUDUO_ACCEPTOR_H
