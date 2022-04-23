# 编译时使用 pthread 库
set(CMAKE_C_FLAGS "-lpthread")

add_executable(Epoll epoll.c)

target_link_libraries(Epoll utils)