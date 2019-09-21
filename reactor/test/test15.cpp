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

std::string message;

void onConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->peerAddress().toHostPort().c_str());

        conn->send(message);
    }
    else
    {
        printf("onConnection(): connection [%s] is down\n",
               conn->name().c_str());
    }
}

void onWriteComplete(const muduo::TcpConnectionPtr& conn)
{
    //conn->send(message);
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

    string line;
    for (int i = 33; i < 127; ++i)
    {
        line.push_back(char(i));
    }
    line += line;
    for (size_t i = 0; i < 127-33; ++i)
    {
        message += line.substr(i, 72) + '\n';
    }

    for (int i = 0; i < 10; ++i)
    {
        message += message;
    }
    //message = "a\n";

    muduo::InetAddress listenAddr(9981);
    EventLoop loop;

    TcpServer server(&loop, listenAddr);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setWriteCompleteCallback(onWriteComplete);
    if (argc > 1)
    {
        server.setThreadNum(atoi(argv[1]));
    }
    server.start();

    loop.loop();

    return 0;
}