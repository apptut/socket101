#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include "socket.h"

int main() {
    char *ip = "0.0.0.0";
    int port = 8888;

    int sockFD = Socket(ip, port);
    if (sockFD < 0) {
        perror("socket init error");
        return sockFD;
    }

    printf("socket init success %s:%d\n", ip, port);

    struct sockaddr_in requestAddr;
    socklen_t len = sizeof(requestAddr);
    int conn = accept(sockFD, (struct sockaddr *) &requestAddr, &len);

    if (conn < 0) {
        perror("accept request socket error");
        exit(EXIT_FAILURE);
    }

    // 处理客户数连接，收发数据
    int max_len = MAX_LEN;
    char buff[MAX_LEN];
    size_t numRead;

    while ((numRead = read(conn, buff, max_len)) > 0) {
        // 把获取的数据写回去
        if (write(conn, buff, numRead) != numRead) {
            perror("send data error");
            return -1;
        }
    }

    if (numRead == 0) {
        printf("the client conn is closed");
    }

    if (numRead == -1) {
        perror("conn error");
        exit(EXIT_FAILURE);
    }

    // 进程退出会自动关闭，并非必要调用close函数
    close(conn);
    close(sockFD);
}
