cmake_minimum_required(VERSION 3.13)
project(mymuduo C CXX)

set(CMAKE_CXX_STANDARD 14)

SET(s00_srcs
    EventLoop.cpp
    EventLoop.h
)

ADD_LIBRARY(s00 ${s00_srcs})
target_link_libraries(s00 s01 s02)

add_executable(s00test1 test1.cpp)
target_link_libraries(s00test1 s00 base s01 pthread)
add_executable(s00test2 test2.cpp)
target_link_libraries(s00test2 s00 base s01 pthread rt)

