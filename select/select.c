#include <stdio.h>
#include <sys/select.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "socket.h"
#include <pthread.h>

fd_set workingSet, masterSet;

int handleConn(int fd);

int main() {
    // 初始化可写集合
    FD_ZERO(&masterSet);

    int fd = Socket("0.0.0.0", 9999);
    // 全局fd句柄扔到监听集合中
    FD_SET(fd, &masterSet);

    int max_fd = fd;
    struct timeval timeout;
    timeout.tv_sec = 3 * 60; // fd监控设置3分钟超时时间
    timeout.tv_usec = 0;

    for (;;) {
        // 复制masterSet的fd到workingSet一份，之所以要复制，是因为workingSet被内核监听后，workingSet数组未激活的fd位会被置位0
        memcpy(&workingSet, &masterSet, sizeof(masterSet));

        printf("等待select \n");
        int rel = select(max_fd + 1, &workingSet, NULL, NULL, &timeout);
        if (rel < 0) {
            perror("select error");
            break;
        }

        printf("select监听发生变化 \n");
        // 遍历检测描述符是否有读操作
        for (int i = 0; i <= max_fd; ++i) {
            if (FD_ISSET(i, &workingSet)) {
                // 如果激活的描述符是socket初始化fd, 那么尝试获取连接fd
                if (i == fd) {
                    struct sockaddr_in request_addr;
                    socklen_t len = sizeof(request_addr);
                    printf("等待客户端连接 \n");
                    int conn_fd = accept(fd, (struct sockaddr *) &request_addr, &len);
                    printf("收到conn连接 \n");
                    if (conn_fd < 0) {
                        perror("connect error");
                        continue;
                    }

                    // 连接fd加入到监听的集合中去
                    FD_SET(conn_fd, &masterSet);

                    if (conn_fd > max_fd) {
                        max_fd = conn_fd;
                    }
                } else {
                    // 被激活的是连接fd, 需要处理当前的连接
                    printf("连接fd达到可用状态: %d \n", i);

                    // 处理客户数连接，收发数据
                    handleConn(i);

                    // 如果连接数据处理完成，则需要清理masterSet中的文件句柄
                    FD_CLR(i, &masterSet);
                    if (i == max_fd) {
                        while (FD_ISSET(max_fd, &masterSet) <= 0) {
                            max_fd -= 1;
                        }
                    }
                }
            }
        }
    }

    printf("server over");
}

/**
 * 处理连接数据
 * @param fd
 * @return
 */
int handleConn(int fd) {
    int max_len = MAX_LEN;
    char buff[MAX_LEN];
    size_t numRead;
    while ((numRead = read(fd, buff, max_len)) > 0) {
        // 把获取的数据写回去
        if (write(fd, buff, numRead) != numRead) {
            perror("send data error");
            return -1;
        }
    }

    if (numRead == 0) {
        printf("the client conn is closed");
    }

    return 0;
}