cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)

set(base_src
        Timestamp.cpp
        Timestamp.h
        copyable.h
        Types.h)

add_library(base ${base_src})