cmake_minimum_required(VERSION 3.13)
project(mymuduo C CXX)

set(CMAKE_CXX_STANDARD 14)

SET(reactor_src
        EventLoop.cpp
        EventLoop.h
        Channel.h
        Channel.cpp
        Poller.h
        Poller.cpp
        Timer.h
        TimerId.h
        Callbacks.h
        TimerQueue.h
        TimerQueue.cpp
        Timer.cpp
        EventLoopThread.cpp
        EventLoopThread.h
        Acceptor.cpp
        Acceptor.h
        Socket.cpp
        Socket.h
        InetAddress.cpp
        InetAddress.h
        SocketsOps.h
        SocketsOps.cpp
        TcpConnection.cpp
        TcpConnection.h
        TcpServer.cpp
        TcpServer.h
        Buffer.cpp
        Buffer.h
        EventLoopThreadPool.cpp
        EventLoopThreadPool.h
        Connector.cpp
        Connector.h
        TcpClient.cpp
        TcpClient.h
        EPoller.cpp
        EPoller.h
        )

add_library(reactor ${reactor_src})
target_link_libraries(reactor base)

