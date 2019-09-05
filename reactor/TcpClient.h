//
// Created by jxq on 19-9-5.
//

#ifndef MYMUDUO_TCPCLIENT_H
#define MYMUDUO_TCPCLIENT_H


#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include "TcpConnection.h"

namespace muduo
{

class Connector;
typedef boost::shared_ptr<Connector> ConnectorPtr;

class TcpClient : boost::noncopyable{
public:
    TcpClient(EventLoop* loop,
            const InetAddress& serverAddr);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr connection() const
    {
        MutexLockGuard lock(mutex_);
        return connection_;
    }

    bool retry() const;
    void enableRetry()
    {
        retry_ = true;
    }

    /// Set connection callback.
    /// Not thread safe.
    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    /// Set message callback.
    /// Not thread safe.
    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    /// Set write complete callback.
    /// Not thread safe.
    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

private:
    /// Not thread safe, but in loop
    void newConnection(int sockfd);
    /// Not thread safe, but in loop
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    ConnectorPtr connector_;    // avoid reveling Connector;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    bool retry_;    // atomic
    bool connect_;  // atomic
    // always in loop thread
    int nextConnId_;
    mutable MutexLock mutex_;
    TcpConnectionPtr connection_ GUARDED_BY(mutex_);
};

}


#endif //MYMUDUO_TCPCLIENT_H
