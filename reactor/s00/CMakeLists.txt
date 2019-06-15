cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread" )

SET(s00_srcs
    EventLoop.cpp
    EventLoop.h
)

include_directories(
        ${PROJECT_SOURCE_DIR}/thread
)



find_package(Boost REQUIRED COMPONENTS regex)
if (NOT Boost_FOUND)
    message("Not found Boost")
endif ()

link_libraries(${Boost_LIBRARIES})
message("${Boost_LIBRARIES}")

ADD_LIBRARY(s00 SHARED ${s00_srcs})
target_link_libraries(s00 thread_srcs)
SET_TARGET_PROPERTIES(s00  PROPERTIES    #表示生成的执行文件所在路径
        RUNTIME_OUTPUT_DIRECTORY> "${PROJECT_SOURCE_DIR}/s00")

