//
// Created by tang on 19-4-2.
//
#include "lept_http.h"
#include "../lept_epoll/lept_epoll.h"
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>

void test_process_request(void)
{
    lept_http_request_t *request = (lept_http_request_t *)malloc(sizeof(lept_http_request_t));
    int epfd = lept_epoll_create();
    int fd = open("/home/tang/CLionProjects/LeptServer/request", O_RDWR);
    lept_http_request_init(request, epfd, fd);
    process_request(request);
}

int main(void)
{
    test_process_request();
}
