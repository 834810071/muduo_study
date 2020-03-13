cmake_minimum_required(VERSION 3.13)
project(mymuduo C CXX)

set(CMAKE_CXX_STANDARD 14)

add_executable(asio_tutorial_timer2 timer2/timer.cc)
add_executable(asio_tutorial_timer3 timer3/timer.cc)
add_executable(asio_tutorial_timer4 timer4/timer.cc)
add_executable(asio_tutorial_timer5 timer5/timer.cc)
add_executable(asio_tutorial_timer6 timer6/timer.cc)


target_link_libraries(asio_tutorial_timer2 reactor)
target_link_libraries(asio_tutorial_timer3 reactor)
target_link_libraries(asio_tutorial_timer4 reactor)
target_link_libraries(asio_tutorial_timer5 reactor)
target_link_libraries(asio_tutorial_timer6 reactor)