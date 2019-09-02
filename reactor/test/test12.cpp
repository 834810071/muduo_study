//
// Created by jxq on 19-9-1.
//

//
// Created by jxq on 19-8-29.
//

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "../InetAddress.h"
#include "../EventLoop.h"
#include "../TcpServer.h"

using namespace std;
using namespace muduo;

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toHostPort().c_str());
    }
    else
    {
        printf("onConnection(): connection [%s] is down\n",
               conn->name().c_str());
    }
}

void onMessage(const TcpConnectionPtr& conn,
               muduo::Buffer* buf,
               Timestamp receiveTime)
{
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
           buf->readableBytes(), conn->name().c_str(), receiveTime.toFormattedString().c_str());

    //printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
    conn->send(buf->retrieveAsString());
}

int main()
{
    printf("main(): pid = %d\n", getpid());

    muduo::InetAddress listenAddr(9981);
    EventLoop loop;

    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();

    return 0;
}