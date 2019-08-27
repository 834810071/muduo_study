cmake_minimum_required(VERSION 3.13)
project(mymuduo C CXX)

set(CMAKE_CXX_STANDARD 14)

set(s02Src
        Timer.h
        TimerId.h
        Callbacks.h
        TimerQueue.h
        TimerQueue.cpp
        Timer.cpp)

add_library(s02 ${s02Src})
target_link_libraries(s02 s01)
add_executable(s02test01 test01.cpp)
target_link_libraries(s02test01 s02 s01 s00 base rt pthread)

add_executable(s02test02 test02.cpp)
target_link_libraries(s02test02 s02 s01 s00 base rt pthread)

add_executable(s02test03 test03.cpp)
target_link_libraries(s02test03 s02 s01 s00 base rt pthread)