cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)

set(ChannelSrc Channel.h
        Channel.cpp
        Poller.h
        Poller.cpp)

add_library(Channelsrc ${ChannelSrc})
add_executable(s01test1 test1.cpp)

target_link_libraries(s01test1 Channelsrc base)