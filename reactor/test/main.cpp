//
// Created by jxq on 19-9-18.
//

//
// 不管什么HTTP请求发过来,永远发送一个HTTP200的报文 -- Boyfriend Web Server
//
// 用于压力测试网络库性能

#include "../TcpServer.h"
#include "../InetAddress.h"
#include "../EventLoop.h"

#include <string>


//using namespace std::string;
using namespace muduo;

std::string g_ServerMsg;

void onConnection(const TcpConnectionPtr& conn){
    if(conn->connected()) {
        //printf("New Connection from %s\n", conn->peerAddress().toIpPort().c_str());
        ;
    }
    else { //已经关闭连接,此时连接断开了
        //printf("Connetion from %s is Down!\n", conn->peerAddress().toIpPort().c_str());
        ;
    }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp recvTime) {
    //收到一条消息,不管是什么,回复一个200OK

    conn->send(g_ServerMsg);
}

void onWriteComplete(const TcpConnectionPtr& conn) {
    conn->shutdown();// 发送完毕直接关闭连接
}

int main() {

    std::string Content("Everything is OK!");
    g_ServerMsg += "HTTP/1.0 200 OK\r\n";
    g_ServerMsg += "Server: Boyfriend Server\r\n"; /* No matter what you said, I always response YES!*/
    g_ServerMsg += "Content-length: "  + std::to_string(Content.size())  + "\r\n";
    g_ServerMsg += "Content-type: text/html\r\n\r\n";
    g_ServerMsg += Content;

    EventLoop loop;
    InetAddress listenAddr(8888);
    TcpServer server(&loop, listenAddr);

    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.setWriteCompleteCallback(onWriteComplete);
    server.setThreadNum(2); //双线程

    server.start();

    loop.loop();

    return 0;
}