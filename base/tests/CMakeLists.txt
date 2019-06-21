cmake_minimum_required(VERSION 3.13)
project(mymuduo)

set(CMAKE_CXX_STANDARD 14)



add_executable(bsa bsa.cpp)
add_executable(Tprint Tprint.cpp)
add_executable(Timestame_unittest Timestame_unittest.cpp)
add_executable(Atomic_unittest Atomic_unittest.cpp)
add_executable(Exception_test Exception_test.cpp)
target_link_libraries(Timestame_unittest base)
target_link_libraries(Atomic_unittest base)
target_link_libraries(Exception_test base)