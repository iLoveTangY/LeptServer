//
// Created by tang on 19-3-19.
//

#include "lept_epoll.h"
#include "../lept_utils/lept_utils.h"
#include "../lept_definition.h"

int lept_epoll_create()
{
    int epfd = epoll_create1(0);
    CHECK(epfd > 0, "Error in epoll_create.");

    events = (lept_epoll_event *)malloc(sizeof(lept_epoll_event) * MAXEVENTS);  // 之前这里写漏了' * MAXEVENTS '，浪费了我两天时间来找这个BUG...
    CHECK(events != NULL, "Error when malloc memory to epoll events.");

    return epfd;
}

int lept_epoll_add(int epfd, int fd, lept_epoll_event *event)
{
    debug("Add %d to epoll", fd);
    int rc = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, event);
    CHECK(rc == 0, "Error in epoll_ctl.");
    return rc;
}

int lept_epoll_mod(int epfd, int fd, lept_epoll_event *event)
{
    debug("Mod %d in epoll", fd);
    int rc = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event);
    CHECK(rc == 0, "Error in epoll_ctl.");
    return rc;
}

int lept_epoll_del(int epfd, int fd, lept_epoll_event *event)
{
    int rc = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, event);
    CHECK(rc == 0, "Error in epoll_ctl.");
    debug("Delete fd %d in epoll", fd);
    return rc;
}

int lept_epoll_wait(int epfd, lept_epoll_event *events, int maxevents, int timeout)
{
    return epoll_wait(epfd, events, maxevents, timeout);;
}
