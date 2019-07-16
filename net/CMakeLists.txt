cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)

set(net_src
        EventLoop.cpp
        EventLoop.h
        TimerId.h
        Callbacks.h
        Channel.h
        Channel.cc
        Poller.cc
        Poller.h
        TimerQueue.cc
        TimerQueue.h
        Endian.h
        Timer.h
        Timer.cc
        SocketsOps.cc
        SocketsOps.h)

add_library(net ${net_src})
target_link_libraries(net pthread base)
