# 编译时使用 pthread 库
set(CMAKE_C_FLAGS "-lpthread")

add_executable(socket01 socket01.c)

target_link_libraries(socket01 utils)