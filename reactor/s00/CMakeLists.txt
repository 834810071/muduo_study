cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)

SET(s00_srcs
    EventLoop.cpp
    EventLoop.h
)


# boost
find_package(Boost REQUIRED COMPONENTS regex)
if (NOT Boost_FOUND)
    message("Not found Boost")
endif ()

link_libraries(${Boost_LIBRARIES})
message("${Boost_LIBRARIES}")


ADD_LIBRARY(s00 SHARED ${s00_srcs})

#target_link_libraries(s00 base)

SET_TARGET_PROPERTIES(s00  PROPERTIES    #表示生成的执行文件所在路径
        RUNTIME_OUTPUT_DIRECTORY> "${PROJECT_SOURCE_DIR}/s00")

add_executable(s00test1 test1.cpp)
target_link_libraries(s00test1 s00 base)
add_executable(s00test2 test2.cpp)
target_link_libraries(s00test2 s00 base)

