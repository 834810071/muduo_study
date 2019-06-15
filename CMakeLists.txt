cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)

ADD_SUBDIRECTORY(reactor/s00)
ADD_SUBDIRECTORY(thread)
ADD_SUBDIRECTORY(base/tests)
ADD_SUBDIRECTORY(base)

add_executable(mymuduo main.cpp)