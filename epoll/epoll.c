#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include "sys/epoll.h"
#include "socket.h"
#include <pthread.h>

#define MAX_EVENTS 16
#define BUF_SIZE 4096

typedef struct {
    int conn_fd;
} conn_param_t;

static int setnonblocking(int sockfd) {
    if (fcntl(sockfd, F_SETFD, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) ==
        -1) {
        return -1;
    }
    return 0;
}

static void epoll_ctl_add(int epfd, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void *connRoutines(void *args) {
    pthread_detach(pthread_self());

    char buf[BUF_SIZE];
    conn_param_t *params = (conn_param_t *) args;

    for (;;) {
        bzero(buf, sizeof(buf));
        ssize_t rel = read(params->conn_fd, buf, sizeof(buf));
        // 成功返回正整数，错误返回 -1
        if (rel <= 0) {
            break;
        }

        // 获取到数据
        printf("[+] data: %s\n", buf);
        write(params->conn_fd, buf, strlen(buf));
    }

    // 释放malloc产生的内存
    free(params);

    return NULL;
}

/**
 * 处理连接数据
 * @param conn_fd
 */
void handleConnect(int conn_fd) {
    pthread_t pid;
    conn_param_t *p = (conn_param_t *) malloc(sizeof(conn_param_t));
    p->conn_fd = conn_fd;

    pthread_create(&pid, NULL, connRoutines, (void *) p);
}

int main() {
    const char *ip = "0.0.0.0";
    int port = 9999;
    int listen_fd = Socket(ip, port);

    // 设置socket为非阻塞模式
    setnonblocking(listen_fd);

    // epoll初始化，size参数内核会忽略，随便填一个大于0的值即可
    struct epoll_event events[MAX_EVENTS];
    int efd = epoll_create(1);

    // 返回fd如果小于0，则创建失败
    if (efd < 0) {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }

    int nfds;
    // 把listen socket 添加到epoll监听列表中，监听读、写事件，边沿模式。
    epoll_ctl_add(efd, listen_fd, EPOLLIN | EPOLLOUT | EPOLLET);

    for (;;) {
        // 超时设置为-1， 没有事件则一直阻塞，内核传递给当前程序的事件，事件不能超过最大的事件数量
        printf("epoll_wait ... \n");

        nfds = epoll_wait(efd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            printf("error: %s errno: %d \n", strerror(errno), errno);
            continue;
        }

        printf("fd found total: %d \n", nfds);

        // 遍历所有ready的描述符数量
        for (int i = 0; i < nfds; ++i) {
            // 如果是listen socket, 获取具体connect socket
            if (events[i].data.fd == listen_fd) {
                struct sockaddr_in request_addr;
                socklen_t len = sizeof(request_addr);
                int conn_sock = accept(listen_fd, (struct sockaddr *) &request_addr, &len);
                if (conn_sock < 0) {
                    perror("accept() error");
                    continue;
                }

                // 设置连接socket非阻塞
                setnonblocking(conn_sock);

                epoll_ctl_add(efd, conn_sock, EPOLLIN | EPOLLET | EPOLLRDHUP);
            } else {
                if (events[i].events & EPOLLIN) {
                    printf("client connected! fd is %d \n", events[i].data.fd);
                    handleConnect(events[i].data.fd);
                } else if (events[i].events & (EPOLLHUP | EPOLLRDHUP)) {
                    printf("[+] connection closed: %d \n", events[i].data.fd);

                    epoll_ctl(efd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                }
            }
        }
    }

    // 关闭fd
    close(listen_fd);
    close(efd);
}