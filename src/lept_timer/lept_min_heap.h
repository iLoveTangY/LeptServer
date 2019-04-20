//
// Created by tang on 19-3-18.
//

#ifndef LEPTSERVER_LEPT_MIN_HEAP_H
#define LEPTSERVER_LEPT_MIN_HEAP_H

#include <stddef.h>

// TODO 基础库不要直接结束程序，而是应该返回错误码

// 比较函数，lhs > rhs 返回 大于0    lhs == rhs 返回0   lhs < rhs 返回小于0
typedef int (*comparator_t)(const void *lhs, const void *rhs);

typedef struct
{
    void **p_data;  // 指向元素数组的指针（数组中每个元素都是指向真正元素的指针）
    size_t size;  // 小根堆当前元素个数
    size_t capacity;  // 小根堆的容量
    comparator_t comparator;  // 元素的比较器
} lept_min_heap_t;

// 初始化小根堆
void min_heap_init(lept_min_heap_t *, comparator_t);

// 往小根堆中添加元素
void min_heap_add(lept_min_heap_t *, void *);

// 删除堆顶元素
void min_heap_del(lept_min_heap_t *);

// 判断小根堆是否为空
int min_heap_is_empty(lept_min_heap_t *);

// 获取小根堆堆顶元素
void* min_heap_top(lept_min_heap_t *);

#endif //LEPTSERVER_LEPT_MIN_HEAP_H
