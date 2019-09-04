//
// Created by jxq on 19-9-3.
//

#ifndef MYMUDUO_CONNECTOR_H
#define MYMUDUO_CONNECTOR_H


#include "EventLoop.h"
#include "InetAddress.h"
#include "TimerId.h"
#include "Callbacks.h"

namespace muduo
{

class EventLoop;
class Channel;

class Connector {
public:
    typedef boost::function<void (int socket)> NewConnectionCallback;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        newConnectionCallback_ = cb;
    }

    void start();  // can be called in any thread
    void restart();  // must be called in loop thread
    void stop();  // can be called in any thread

    const InetAddress& serverAddress() const { return serverAddr_; }

private:
    enum States {kDisconnected, kConnecting, kConnected};
    static const int kMaxRetryDelayMs = 30*1000;
    static const int kInitRetryDelayMs = 500;

    void setState(States s)
    {
        state_ = s;
    }
    void startInLoop();
    void connect();
    void connecting(int sockfd);
    void handleWrite();
    void handleError();
    void retry(int sockfd);
    int removeAndResetChannel();
    void resetChannel();

    EventLoop* loop_;
    InetAddress serverAddr_;
    bool connect_;  // atomic
    States state_;  // FIXME: use atomic variable
    boost::scoped_ptr<Channel> channel_;
    NewConnectionCallback newConnectionCallback_;
    int retryDelayMs_;
   // TimerId timerId_;
};

typedef boost::shared_ptr<Connector> ConnectorPtr;
}


#endif //MYMUDUO_CONNECTOR_H
