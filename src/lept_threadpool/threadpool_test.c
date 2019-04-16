//
// Created by tang on 19-3-22.
//
#include <stdio.h>
#include <assert.h>
#include "lept_thread_pool.h"
#include "../lept_definition.h"
#include "../lept_utils/lept_utils.h"

pthread_mutex_t lock;
size_t sum = 0;

static void sum_n(void *arg)
{
    size_t n = (size_t) arg;
    int rc = pthread_mutex_lock(&lock);

    CHECK(rc == 0, "Error in line 15");

    sum += n;

    rc = pthread_mutex_unlock(&lock);
    CHECK(rc == 0, "Error in line 21");
}

int main()
{
    int rc = pthread_mutex_init(&lock, NULL);
    CHECK(rc == 0, "Error in line 27");
    lept_thread_pool_t *tp = (lept_thread_pool_t *)malloc(sizeof(lept_thread_pool_t));
    lept_threadpool_init(tp, 8);
    CHECK(tp != NULL, "error in line 31");

    size_t i;
    for (i = 0; i < 1000; ++i)
    {
        rc = lept_threadpool_add(tp, sum_n, (void *)i);
        CHECK(rc == 0, "Error in line 36");
    }

    rc = lept_threadpool_destroy(tp, GRACEFUL_SHUTDWON);
    CHECK(rc == 0, "Error in line 41");

    printf("sum: %d\n", (int)sum);
    assert(sum == 499500);

    printf("OK!\n");

    return 0;
}
