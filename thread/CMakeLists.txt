cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -O3 -pthread" )


SET(thread_srcs
    ./Thread.cc
        ./Thread.h
        ./Atomic.h
)

include_directories(
        ${PROJECT_SOURCE_DIR}/reactor/s00

)

find_package (Threads REQUIRED)

find_package(Boost REQUIRED COMPONENTS regex)
if (NOT Boost_FOUND)
    message("Not found Boost")
endif ()

link_libraries(${Boost_LIBRARIES})
message("${Boost_LIBRARIES}")

ADD_LIBRARY(thread SHARED ${thread_srcs})

SET_TARGET_PROPERTIES(thread  PROPERTIES    #表示生成的执行文件所在路径
        RUNTIME_OUTPUT_DIRECTORY> "${PROJECT_SOURCE_DIR}/thread")