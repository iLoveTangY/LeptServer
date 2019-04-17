//
// Created by tang on 19-3-18.
//

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include "../lept_utils/lept_utils.h"
#include "lept_thread_pool.h"
#include "../lept_definition.h"

static void *thread(void *argp);

int lept_threadpool_init(lept_thread_pool_t* pool, int thread_num)
{
    if (pool == NULL)
        return -1;
    if (thread_num <= 0)
        return -1;
    int rc;
    rc = pthread_mutex_init(&pool->lock, NULL);
    CHECK(rc == 0, "Error in mutex init.");
    pool->thread_count = 0;
    pool->started = 0;
    pool->head = (lept_task_t *)malloc(sizeof(lept_task_t));  // 链表的dummy head
    CHECK(pool->head != NULL, "Error when malloc memory to pool->head.");
    pool->head->next = NULL;
    pool->queue_size = 0;
    pool->shutdown = 0;
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_num);
    CHECK(pool->threads != NULL, "Error when malloc memory to pool->threads");
    rc = pthread_cond_init(&pool->cond, NULL);
    CHECK(rc == 0, "Error when init cond variable.");

    for (int i = 0; i < thread_num; ++i)
    {
        rc = pthread_create(pool->threads+i, NULL, thread, (void *)pool);
        CHECK(rc == 0, "Error in pthread_create");

        pool->thread_count++;
        pool->started++;

        log_info("Thread id: %lu created\n", *((pool->threads) + i));
    }

    return 0;
}

int lept_threadpool_add(lept_thread_pool_t *pool, void (*p_function)(void *), void *arg)
{
    CHECK(pool != NULL && p_function != NULL, "thread pool and function can't be NULL.");

    int rc = pthread_mutex_lock(&(pool->lock));
    CHECK(rc == 0, "Error in lock mutex.");
    if (pool->shutdown)
    {
        rc = pthread_mutex_unlock(&(pool->lock));
        CHECK(rc == 0, "Error in unlock mutex");
    }

    lept_task_t *task = (lept_task_t *)malloc(sizeof(lept_task_t));
    CHECK(task != NULL, "Error when malloc memory to lept_task.");

    task->p_function = p_function;
    task->args = arg;
    task->next = pool->head->next;
    pool->head->next = task;

    pool->queue_size++;

    debug("Main Thread: send signal to cond variable");
    rc = pthread_cond_signal(&(pool->cond));
    CHECK(rc == 0, "Error in pthread_cond_signal.");

    rc = pthread_mutex_unlock(&(pool->lock));
    CHECK(rc == 0, "Error in unlock mutex.");

    return 0;
}

void lept_threadpool_free(lept_thread_pool_t *pool)
{
    if (pool == NULL || pool->started > 0)
    {
        log_info("Empty Pool.");
        return;
    }

    if (pool->threads)
        free(pool->threads);

    lept_task_t *tmp;
    while (pool->head->next)  // 依次释放链表节点
    {
        tmp = pool->head->next;
        pool->head->next = pool->head->next->next;
        free(tmp);
    }
    free(pool->head);  // 释放dummy_head
}

int lept_threadpool_destroy(lept_thread_pool_t *pool, int graceful)
{
    if (pool == NULL)
    {
        fprintf(stderr, "empty pool");
        return -1;
    }

    pthread_mutex_lock(&(pool->lock));  // TODO 检查返回值

    if (pool->shutdown)
    {
        fprintf(stderr, "already shutdown");
        return -1; // 返回错误代码
    }

    pool->shutdown = (graceful) ? GRACEFUL_SHUTDWON : IMMEDIATE_SHUTDOWN;

    pthread_cond_broadcast(&(pool->cond)); // TODO 返回值判断
    pthread_mutex_unlock(&(pool->lock)); // TODO

    for (int i = 0; i < pool->thread_count; ++i)
    {
        if (pthread_join(pool->threads[i], NULL) != 0)
            return -1;
    }

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->cond));
    lept_threadpool_free(pool);

    return 0;
}

static void *thread(void *argp)
{
    if (argp == NULL)
    {
        // TODO
        return NULL;
    }

    lept_thread_pool_t *pool = (lept_thread_pool_t *)argp;

    for (;;)
    {
        // TODO 判断返回值
        pthread_mutex_lock(&(pool->lock));
        while ((pool->queue_size == 0) && !(pool->shutdown))
        {
            // TODO 判断返回值
            debug("Thread %lu Waiting for cond...", pthread_self());
            pthread_cond_wait((&pool->cond), &(pool->lock));
        }

        if (pool->shutdown == IMMEDIATE_SHUTDOWN)
            break;
        else if (pool->shutdown == GRACEFUL_SHUTDWON && pool->queue_size == 0)
            break;

        lept_task_t *task = pool->head->next;
        assert(task != NULL);

        debug("Thread %lu can do something...", pthread_self());
        pool->head->next = task->next;
        --(pool->queue_size);

        pthread_mutex_unlock(&(pool->lock)); // TODO

        (*(task->p_function))(task->args);
        debug("Thread %lu finish task...", pthread_self());
        free(task);
    }

    pool->started--;
    pthread_mutex_unlock(&(pool->lock));  // 从循环中跳出必须解锁
    fprintf(stdout, "Thread id %lu exit.\n", pthread_self());
    pthread_exit(NULL);
}
