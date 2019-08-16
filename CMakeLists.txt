cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)

ADD_SUBDIRECTORY(reactor/s00)
ADD_SUBDIRECTORY(base/tests)
ADD_SUBDIRECTORY(base)
ADD_SUBDIRECTORY(net)
ADD_SUBDIRECTORY(net/test)
ADD_SUBDIRECTORY(book_study/Chapter01)

add_executable(mymuduo main.cpp)