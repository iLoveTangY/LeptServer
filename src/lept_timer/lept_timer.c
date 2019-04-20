//
// Created by tang on 19-4-18.
//

#include <sys/time.h>
#include "lept_timer.h"
#include "../lept_utils/lept_utils.h"


// 获取当前时间，毫秒数
static size_t get_current_time()
{
    struct timeval tv;
    int rc = gettimeofday(&tv, NULL);
    CHECK(rc == 0, "Error in gettime of day");

    size_t current_sec = (size_t) tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return current_sec;
}

static int timer_comp(const void *lhs, const void *rhs)
{
    const lept_timer_node_t *l = (const lept_timer_node_t *)lhs;
    const lept_timer_node_t *r = (const lept_timer_node_t *)rhs;

    return l->timeout > r->timeout;
}

void lept_server_timer_init(lept_server_timers_t *server_timers)
{
    server_timers->min_heap = (lept_min_heap_t *)malloc(sizeof(lept_min_heap_t));
    CHECK(server_timers->min_heap != NULL, "Error when malloc space to server timers");
    min_heap_init(server_timers->min_heap, timer_comp);
}

int get_epoll_wait_time(lept_server_timers_t *server_timers)
{
    int time = -1;
    while (!min_heap_is_empty(server_timers->min_heap))
    {
        lept_timer_node_t *timer_node = min_heap_top(server_timers->min_heap);
        CHECK(timer_node != NULL, "Error in min_heap_top");
        if (timer_node->deleted)
        {
            min_heap_del(server_timers->min_heap);
            free(timer_node);
            continue;
        }
        size_t cur_time = get_current_time();
        time = (int)(timer_node->timeout - cur_time);  // 离超时时间还有多久
        if (time < 0)
            time = 0;  // 如果已经过了超时时间，返回0，令后面的epoll_wait系统调用立即返回
        break;
    }
    return time;
}

void handle_expire_timers(lept_server_timers_t *server_timers)
{
    while (!min_heap_is_empty(server_timers->min_heap))
    {
        lept_timer_node_t *timer_node = min_heap_top(server_timers->min_heap);
        CHECK(timer_node != NULL, "Error in min_heap_top");
        if (timer_node->deleted)
        {
            min_heap_del(server_timers->min_heap);
            free(timer_node);
            continue;
        }
        size_t cur_time = get_current_time();
        if (timer_node->timeout > cur_time)
            return;

        if (timer_node->handler)
            timer_node->handler(timer_node->request);  // 超时连接要关闭
        min_heap_del(server_timers->min_heap);  // 从堆中移除超时的节点
        debug("connection %d is timeout, closed", timer_node->request->fd);
        free(timer_node);
    }
}

void add_timer(lept_server_timers_t *server_timers, lept_http_request_t *request, size_t timeout, lept_timer_handler handler)
{
    lept_timer_node_t *timer_node = (lept_timer_node_t *)malloc(sizeof(lept_timer_node_t));
    CHECK(timer_node != NULL, "Error when malloc to timer node");
    timer_node->timeout = timeout + get_current_time();
    timer_node->handler = handler;
    timer_node->request = request;
    timer_node->deleted = 0;

    request->p_timer_node = timer_node;

    min_heap_add(server_timers->min_heap, timer_node);
}

void del_timer(lept_http_request_t *request)
{
    lept_timer_node_t *timer_node = request->p_timer_node;
    CHECK(timer_node != NULL, "request's timer is NULL");
    timer_node->deleted = 1;  // lazy delete
}
