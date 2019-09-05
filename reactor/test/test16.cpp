//
// Created by jxq on 19-9-4.
//

#include "../Connector.h"
#include "../EventLoop.h"

#include <stdio.h>

muduo::EventLoop* g_loop;

int i = 0;
void connectCallback(int sockfd)
{
    ++i;
    printf("%d : \t", i);
    printf("connected.\n");
    g_loop->quit();
}

int main(int argc, char** argv)
{
    muduo::EventLoop loop;
    g_loop = &loop;

    muduo::InetAddress addr("127.0.0.1", 9981);

    int num = 1;
    if (argc > 1)
    {
        num = atoi(argv[1]);
    }
    vector<muduo::ConnectorPtr> vec;
    for (int i = 0; i < num; ++i)
    {
        muduo::ConnectorPtr connector(new muduo::Connector(&loop, addr));
        vec.push_back(connector);
        vec[i]->setNewConnectionCallback(connectCallback);
        vec[i]->start();
    }

    loop.loop();
}