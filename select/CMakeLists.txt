add_executable(Select select.c)

# 链接utils静态库
target_link_libraries(Select utils)