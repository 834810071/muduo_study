cmake_minimum_required(VERSION 3.13)
project(mymuduo C CXX)

set(CMAKE_CXX_STANDARD 14)

set(s04Src
        Acceptor.cpp
        Acceptor.h
        Socket.cpp
        Socket.h
        InetAddress.cpp
        InetAddress.h
        SocketsOps.h
        SocketsOps.cpp)

add_library(s04 ${s04Src})
target_link_libraries(s04 s03 s02 s01 s00)

add_executable(s04test1 test1.cpp)
target_link_libraries(s04test1 s04 s03 s02 s01 s00 base rt pthread)