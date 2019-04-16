//
// Created by tang on 19-3-18.
//

#ifndef LEPTSERVER_LEPT_THREAD_POOL_H
#define LEPTSERVER_LEPT_THREAD_POOL_H

#include <pthread.h>
#include <semaphore.h>

typedef struct lept_task
{
    void *args;
    void (*p_function)(void *);
    struct lept_task *next;
} lept_task_t;

typedef struct lept_thread_pool
{
    pthread_mutex_t lock;  // 控制对线程池中变量的访问
    pthread_cond_t cond;  // 当有新任务加到线程池中时条件变量发起通知
    int queue_size;  // 线程池中准备好的任务的数量
    int shutdown;
    int thread_count;  // 线程池中线程的数量
    int started;  // 启动了的线程的数量
    pthread_t *threads;
    lept_task_t *head;
} lept_thread_pool_t;

// 初始化线程池
int lept_threadpool_init(lept_thread_pool_t*, int thread_num);

// 向线程池中添加一项任务
int lept_threadpool_add(lept_thread_pool_t *pool, void (*p_function)(void *), void *arg);

// 释放线程池占用的内存
void lept_threadpool_free(lept_thread_pool_t *pool);

// 销毁线程池
int lept_threadpool_destroy(lept_thread_pool_t *pool, int graceful);


#endif //LEPTSERVER_LEPT_THREAD_POOL_H
