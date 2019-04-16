//
// Created by tang on 19-3-18.
//

#ifndef LEPTSERVER_UTILS_H
#define LEPTSERVER_UTILS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "../log.h"

#define ISNUMBER(ch) ((ch) >= '0' && (ch) <= '9')

#define CHECK(A, M, ...) if(!(A)) { log_err(M "\n", ##__VA_ARGS__); exit(1);}

typedef struct runtime_context
{
    int thread_nums;
    char *port_num;
} runtime_context_t;

typedef void handler_t(int);

// 在指定端口上打开一个监听描述符
// csapp
int lept_open_listenfd(const char *port);

// 使指定的描述符变为非阻塞
int lept_make_fd_unblocked(int fd);

// csapp
handler_t *lept_signal(int signo, handler_t *p_func);

int lept_str_n_cmp(const char *m, const char *s, int n);

// csapp
ssize_t rio_writen(int fd, void *usrbuf, size_t n);

#endif //LEPTSERVER_UTILS_H
