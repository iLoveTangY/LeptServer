//
// Created by tang on 19-3-18.
//

#include "lept_min_heap.h"
#include "../lept_utils/lept_utils.h"

static void swap(void **p, size_t i, size_t j)
{
    void *temp = p[i];
    p[i] = p[j];
    p[j] = temp;
}

static void floating_up(lept_min_heap_t *heap, size_t index)
{
    if (index >= heap->size || index == 0)
        return;
    size_t father_index = (index - 1) / 2;
    while (heap->comparator(heap->p_data[father_index], heap->p_data[index]) > 0)
    {
        swap(heap->p_data, father_index, index);
        index = father_index;
        if (index == 0)
            break;
        father_index = (index - 1) / 2;
    }
}

static void sink(lept_min_heap_t *heap, size_t index)
{
    if (index >= heap->size || index == heap->size - 1)
        return;
    size_t left = 2 * index + 1;
    while (left < heap->size)
    {
        size_t largest = (left + 1 < heap->size) &&
                         heap->comparator(heap->p_data[left+1], heap->p_data[left]) > 0 ? left + 1 : left;
        largest = heap->comparator(heap->p_data[largest], heap->p_data[index]) < 0 ? largest : index;
        if (largest == index)
            break;
        swap(heap->p_data, largest, index);
        index = largest;
        left = 2 * index + 1;
    }
}

// 空间不够时扩容或者缩小容量
static void resize(lept_min_heap_t *heap, size_t new_size)
{
    CHECK(heap->size <= new_size, "New size too small");

    void **new_ptr = (void **)malloc(sizeof(void *) * new_size);
    CHECK(new_ptr != NULL, "malloc new size error");

    memcpy(new_ptr, heap->p_data, sizeof(void *) * heap->size);
    free(heap->p_data);
    heap->p_data = new_ptr;
    heap->capacity = new_size;
}

void min_heap_init(lept_min_heap_t *heap, comparator_t comp)
{
    heap->p_data = NULL;
    heap->comparator = comp;
    heap->capacity = 0;
    heap->size = 0;
}

inline int min_heap_is_empty(lept_min_heap_t *heap)
{
    CHECK(heap != NULL, "heap is NULL");
    return heap->size == 0;
}

inline void* min_heap_top(lept_min_heap_t *heap)
{
    CHECK(heap != NULL, "heap is NULL");
    if (heap->size == 0)
        return NULL;
    return heap->p_data[0];  // 返回堆顶元素
}

void min_heap_add(lept_min_heap_t *heap, void *item)
{
    CHECK(heap != NULL, "heap is NULL");
    // 如果空间不够需要增加空间
    if (heap->size == heap->capacity)
    {
        size_t new_size = heap->size == 0 ? 1 : heap->size * 2;
        resize(heap, new_size);
    }
    heap->p_data[heap->size++] = item;
    floating_up(heap, heap->size-1);  // 将最后一个元素上浮
}

void min_heap_del(lept_min_heap_t *heap)
{
    CHECK(heap != NULL, "heap is NULL");
    if (min_heap_is_empty(heap))
        return;
    swap(heap->p_data, 0, --heap->size);  // 交换堆顶元素和最后一个元素
    sink(heap, 0);
    if (heap->size < heap->capacity / 2)
        resize(heap, heap->capacity / 2);
}
