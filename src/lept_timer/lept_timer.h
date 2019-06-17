//
// Created by tang on 19-4-18.
//

#ifndef LEPTSERVER_LEPT_TIMER_H
#define LEPTSERVER_LEPT_TIMER_H

#include <stddef.h>
#include "../lept_http/lept_http.h"
#include "lept_min_heap.h"

// 考虑定时器是否线程安全？
// 定时器不是线程安全的，需要加锁，后面的版本去做

typedef int (*lept_timer_handler)(lept_http_request_t *);

typedef struct
{
    size_t timeout;  // 当前节点超时时间（毫秒）（离1970.1.1的毫秒数）
    int deleted;  // 当前节点是否应该被删除（lazy delete）
    lept_timer_handler handler;  // 超时处理函数
    lept_http_request_t *request;  // 与当前定时器对应的http请求
} lept_timer_node_t;

typedef struct
{
    lept_min_heap_t *min_heap;
} lept_server_timers_t;  // 定时器集合类型

// 初始化一个定时器集合（实际上是小根堆）
void lept_server_timer_init(lept_server_timers_t *);

// 获取离当前时间最近的那个定时器的超时时间
int get_epoll_wait_time(lept_server_timers_t *);

// 处理已超时的定时器（关闭连接）
void handle_expire_timers(lept_server_timers_t *);

// 将一个新的定时器加到定时器集合中
void add_timer(lept_server_timers_t *, lept_http_request_t *, size_t, lept_timer_handler);

// 从定时器集合中删除一个http请求对应的定时器, lazy delete
void del_timer(lept_http_request_t *);

#endif //LEPTSERVER_LEPT_TIMER_H
