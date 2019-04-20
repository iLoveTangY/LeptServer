//
// Created by tang on 19-4-19.
//

#include "lept_min_heap.h"
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

int comp(const void *lhs, const void *rhs)
{
    const int *l = (const int *)lhs;
    const int *r = (const int *)rhs;
    return *l > *r;
}

void test_min_heap_init(lept_min_heap_t *min_heap)
{
    assert(min_heap);
    min_heap_init(min_heap, &comp);
    assert(min_heap->size == 0);
    assert(min_heap->capacity == 0);
    assert(min_heap->p_data == NULL);
}

void test_min_heap_add(lept_min_heap_t *min_heap)
{
    assert(min_heap);
    int *i = (int *)malloc(sizeof(int));
    *i = 10;
    min_heap_add(min_heap, i);
    assert(min_heap->size == 1);
    assert(min_heap->capacity == 1);
    assert(min_heap_is_empty(min_heap) == 0);
    assert(*(int *)min_heap_top(min_heap) == 10);

    int *j = (int *)malloc(sizeof(int));
    *j = 2;
    min_heap_add(min_heap, j);
    assert(min_heap->size == 2);
    assert(min_heap->capacity == 2);
    assert(min_heap_is_empty(min_heap) == 0);
    assert(*(int *)min_heap_top(min_heap) == 2);

    int *k = (int *)malloc(sizeof(int));
    *k = 6;
    min_heap_add(min_heap, k);
    assert(min_heap->size == 3);
    assert(min_heap->capacity == 4);
    assert(min_heap_is_empty(min_heap) == 0);
    assert(*(int *)min_heap_top(min_heap) == 2);
}

void test_min_heap_del(lept_min_heap_t *min_heap)
{
    assert(min_heap);
    void * p= min_heap_top(min_heap);
    assert(*(int *)p == 2);
    free(p);
    min_heap_del(min_heap);
    assert(*(int *)min_heap_top(min_heap) == 6);
    assert(min_heap->size == 2);
    assert(min_heap->capacity == 4);

    p = min_heap_top(min_heap);
    assert(*(int *)p == 6);
    free(p);
    min_heap_del(min_heap);
    assert(*(int *)min_heap_top(min_heap) == 10);
    assert(min_heap->size == 1);
    assert(min_heap->capacity == 2);

    p = min_heap_top(min_heap);
    assert(*(int *)p == 10);
    free(p);
    min_heap_del(min_heap);
    assert(min_heap->size == 0);
    assert(min_heap->capacity == 1);
}

void test_min_heap()
{
    lept_min_heap_t *min_heap = (lept_min_heap_t *)malloc(sizeof(lept_min_heap_t));
    test_min_heap_init(min_heap);
    test_min_heap_add(min_heap);
    test_min_heap_del(min_heap);
}

int main(void)
{
    test_min_heap();

    return 0;
}