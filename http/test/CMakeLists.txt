cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)

add_executable(LoggingTest LoggingTest.cpp)

target_link_libraries(LoggingTest base reactor http)

add_executable(HttpServer_test HttpServer_test.cc)

target_link_libraries(HttpServer_test base reactor http)

#add_executable(HttpRequest_unittest HttpRequest_unittest.cc)
#
#target_link_libraries(HttpRequest_unittest base reactor http)