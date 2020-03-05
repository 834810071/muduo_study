cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)

add_executable(LoggingTest LoggingTest.cpp)

target_link_libraries(LoggingTest base reactor)