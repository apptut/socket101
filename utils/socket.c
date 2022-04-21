#include "socket.h"
#include <stdint.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/**
 * listen and bind socket01
 *
 * @param ip
 * @param port
 * @return fd or -1 if error
 */
int Socket(const char *ip, uint16_t port) {
    printf("init socket server: %s:%d \n", ip, port);

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
