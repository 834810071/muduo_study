cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)

add_executable(Tprint0 Tprint.cpp)

target_link_libraries(Tprint0 net)