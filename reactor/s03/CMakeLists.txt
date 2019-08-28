cmake_minimum_required(VERSION 3.13)
project(mymuduo C CXX)

set(CMAKE_CXX_STANDARD 14)

set(s03src
        EventLoopThread.cpp
        EventLoopThread.h)

add_library(s03 ${s03src})
target_link_libraries(s03 s02 s01 s00)

add_executable(s03test1 test1.cpp)
target_link_libraries(s03test1 s03 s02 s01 s00 base rt pthread)

add_executable(s03test2 test2.cpp)
target_link_libraries(s03test2 s03 s02 s01 s00 base rt pthread)