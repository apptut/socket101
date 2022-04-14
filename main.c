#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <strings.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int fd;
} routine_t;


void *routine(void *args) {
    // 分离线程，待线程结束自动回收线程资源
    pthread_detach(pthread_self());

    routine_t *params = (routine_t *) args;

    char *data = "hello the socket01 \n";

    // 写入数据
    size_t left_len; // 剩余需要写入的字节数
    size_t total_len = strlen(data);
    left_len = total_len;
    
    while (left_len > 0) {
        ssize_t n;
        n = write(params->fd, data, left_len);
        if (n < 0) {
            // 写错误
            perror(strerror(errno));
            break;
        }

        left_len -= n; // 更新剩余发送长度
        data += n; // 偏移指针
    }

    printf("finished \n");

    close(params->fd);

    // write data
    return NULL;
}

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
    sockaddrIn.sin_addr.s_addr = htonl(INADDR_ANY);
    // 绑定ip地址
//    if (inet_pton(AF_INET, ip, &sockaddrIn.sin_addr) <= 0) {
//        return -1;
//    }
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
    const char *ip = "0.0.0.0";
    int port = 20000;

    int fd = Socket(ip, port);
    if (fd < 0) {
        perror(strerror(errno));
        exit(errno);
    }

    /*******************************************************************************************************/
    /* Allow socket01 descriptor to be reuseable                                                             */
    /*******************************************************************************************************/
    int rc, on = 1;
    rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
    if (rc < 0) {
        perror("setsockopt() failed");
        close(fd);
        exit(-1);
    }

    printf("init socket01 server: %s:%d \n", ip, port);


    for (;;) {
        struct sockaddr_in childAddr;
        socklen_t len = sizeof(childAddr);
        int conn = accept(fd, (struct sockaddr *) &childAddr, &len);
        if (conn < 0) {
            perror(strerror(errno));
            break;
        }

        pthread_t p;
        routine_t arg = {.fd=conn};
        pthread_create(&p, NULL, routine, (void *) &arg);
    }

    printf("server exist");

    return 0;
}
