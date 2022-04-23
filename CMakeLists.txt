cmake_minimum_required(VERSION 3.21.6)
project(TCP_01 C)

set(CMAKE_C_STANDARD 99)

add_subdirectory(utils)
add_subdirectory(socket01)
add_subdirectory(select)
add_subdirectory(epoll)