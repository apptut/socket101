cmake_minimum_required(VERSION 3.21.6)
project(TCP_01 C)

set(CMAKE_C_STANDARD 99)

add_subdirectory(select)
add_subdirectory(socket01)

# 编译时使用 pthread 库
set(CMAKE_C_FLAGS "-lpthread")
add_executable(TCP_01 main.c)
