//
// Created by jxq on 19-8-29.
//

#ifndef MYMUDUO_TCPCONNECTION_H
#define MYMUDUO_TCPCONNECTION_H

#include <boost/core/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/any.hpp>
#include "InetAddress.h"
#include "EventLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "Callbacks.h"

namespace muduo
{

class Channel;
class EventLoop;
class Socket;

///
/// TCP connection, for both client and server usage.
///
class TcpConnection : boost::noncopyable,
                      public boost::enable_shared_from_this<TcpConnection>
{
public:
    /// Constructs a TcpConnection with a connected sockfd
    ///
    /// User should not create this object.
    TcpConnection(EventLoop* loop,
            const std::string& name,
            int sockfd,
            const InetAddress& localAddr,
            const InetAddress& peerAddr);

    ~TcpConnection();

    EventLoop* getLoop() const
    {
        return loop_;
    }

    const std::string& name() const
    {
        return name_;
    }

    const InetAddress& localAddress()
    {
        return localAddr_;
    }

    const InetAddress& peerAddress()
    {
        return peerAddr_;
    }

    bool connected() const
    {
        return state_ == kConnected;
    }

    //void send(const void* message, size_t len);
    // Thread safe.
    void send(const std::string& message);
    void send(Buffer* buf);
    // Thread safe.
    void shutdown();
    void setTcpNoDelay(bool on);

    void setConnectionCallback(const ConnectionCallback& cb)
    {
        connectionCallback_ = cb;
    }

    void setMessageCallback(const MessageCallback& cb)
    {
        messageCallback_ = cb;
    }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        writeCompleteCallback_ = cb;
    }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    { highWaterMarkCallback_ = cb; highWaterMark_ = highWaterMark; }

    /// Internal use only.
    void setCloseCallback(const CloseCallback& cb)
    {
        closeCallback_ = cb;
    }

    // called when TcpServer accepts a new connection
    void connectEstablished();   // should be called only once
    // called when TcpServer has removed me from its map
    void connectDestroyed();  // should be called only once

    void setContext(const boost::any& context)
    { context_ = context; }

    const boost::any& getContext() const
    { return context_; }

    boost::any* getMutableContext()
    { return &context_; }

private:
    enum StateE {kConnecting, kConnected, kDisconnecting, kDisconnected};

    void setState(StateE s)
    {
        state_ = s;
    }

    void handleRead(Timestamp receiveTime);
    void handleError();
    void handleWrite();
    void handleClose();
    void sendInLoop(const std::string& message);
    void sendInLoop(const void* message, size_t len);
    void shutdownInLoop();

    EventLoop* loop_;
    std::string name_;
    StateE state_;  // FIXME: use atomic variable
    // we don't expose those classes to client
    boost::scoped_ptr<Socket> socket_;
    boost::scoped_ptr<Channel> channel_;
    InetAddress localAddr_;
    InetAddress peerAddr_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;
    boost::any context_;
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

}


#endif //MYMUDUO_TCPCONNECTION_H
