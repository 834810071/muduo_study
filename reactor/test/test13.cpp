//
// Created by jxq on 19-9-2.
//

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
#include <signal.h>

using namespace std;
using namespace muduo;

std::string message1;
std::string message2;
unsigned int sleepSeconds = 5;

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toHostPort().c_str());

        if (sleepSeconds > 0)
        {
            ::sleep(sleepSeconds);
        }
        conn->send(message1);
        conn->send(message2);
        conn->shutdown();
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

    printf("onMessage(): [%s]\n", buf->retrieveAsString().c_str());
    //buf->retrieveAll();
}

int main(int argc, char** argv)
{
    printf("main(): pid = %d\n", getpid());

    int len1 = 1;
    int len2 = 2;

    if (argc > 2)
    {
        len1 = atoi(argv[1]);
        len2 = atoi(argv[2]);
    }

    message1.resize(len1);
    message2.resize(len2);
    std::fill(message1.begin(), message1.end(), 'A');
    std::fill(message2.begin(), message2.end(), 'B');

    muduo::InetAddress listenAddr(9981);
    EventLoop loop;

    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.start();

    loop.loop();

    return 0;
}