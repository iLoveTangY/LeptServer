//
// Created by tang on 19-3-19.
//

#ifndef LEPTSERVER_LEPT_EPOLL_H
#define LEPTSERVER_LEPT_EPOLL_H

#include <sys/epoll.h>

typedef struct epoll_event lept_epoll_event;

lept_epoll_event *events;

int lept_epoll_create();

int lept_epoll_add(int epfd, int fd, lept_epoll_event *event);

int lept_epoll_mod(int epfd, int fd, lept_epoll_event *event);

int lept_epoll_del(int epfd, int fd, lept_epoll_event *event);

int lept_epoll_wait(int epfd, lept_epoll_event *events, int maxevents, int timeout);

#endif //LEPTSERVER_LEPT_EPOLL_H
