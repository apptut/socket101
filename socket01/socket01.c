#include <stdint.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_LEN 4096

/**
 * listen and bind socket01
 *
 * @param ip
 * @param port
 * @return fd or -1 if error
 */
int Socket(const char *ip, uint16_t port) {
    /*******************************************************************************/
    /* 初始化 socket句柄                                                             */
    /*******************************************************************************/
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket01 init error");
        exit(1);
    }
    // bind ip , port
    struct sockaddr_in sockaddrIn;
    struct sockaddr_in6 sockaddrIn6;
    bzero(&sockaddrIn, sizeof(sockaddrIn));

    // 配置端口号, 大端网络序
    sockaddrIn.sin_port = htons(port);
    sockaddrIn.sin_family = AF_INET;
//    sockaddrIn.sin_addr.s_addr = htonl(INADDR_ANY);

    // 绑定ip地址
    if (inet_pton(AF_INET, ip, &sockaddrIn.sin_addr) <= 0) {
        return -1;
    }

    int rel = bind(fd, (struct sockaddr *) &sockaddrIn, sizeof(sockaddrIn));
    if (rel < 0) {
        return -1;
    }

    // listen
    rel = listen(fd, SOMAXCONN);
    if (rel < 0) {
        perror("socket01 listen error");
        return rel;
    }

    return fd;
}


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
