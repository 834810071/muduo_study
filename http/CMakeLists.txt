cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)

set(http_src
        HttpContext.cc
        HttpContext.h
        HttpRequest.h
        HttpResponse.cc
        HttpResponse.h
        HttpServer.cc
        HttpServer.h)

add_library(http ${http_src})
target_link_libraries(http reactor)