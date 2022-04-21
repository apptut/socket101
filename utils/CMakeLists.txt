cmake_minimum_required(VERSION 3.21.6)

# 默认位静态库
add_library(utils socket.c)

# 添加当前库的头文件检测目录
target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})