cmake_minimum_required(VERSION 3.13)
project(mymuduo C CXX)

set(CMAKE_CXX_STANDARD 14)


set(s05Src
        TcpConnection.cpp
        TcpConnection.h
        TcpServer.cpp
        TcpServer.h
        Callbacks.h)

add_library(s05 ${s05Src})
target_link_libraries(s05 s04 s03 s02 s01 s00)

add_executable(s05test1 test1.cpp)
target_link_libraries(s05test1 s05 s04 s03 s02 s01 s00 base rt pthread)