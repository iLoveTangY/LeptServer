//
// Created by tang on 19-6-16.
//

#ifndef LEPTSERVER_CURRENTTHREAD_H
#define LEPTSERVER_CURRENTTHREAD_H

#include <sys/types.h>  // for pid_t

namespace CurrentThreadInfo  // 当前线程的线程信息
{
    extern __thread int cached_tid;  // pid_t形式的线程id
    extern __thread char tid_string[32];  // 字符串形式的线程id（__thread）无法修饰class类型，只能用char[]
    extern __thread const char *thread_name;  // 线程名称

    pid_t get_tid();

    void cache_tid();

    inline int tid()
    {
        if (__builtin_expect(cached_tid == 0, 0))
            cache_tid();
        return cached_tid;
    }

    inline const char* get_tid_string()
    {
        return tid_string;
    }

    inline const char* get_thread_name()
    {
        return thread_name;
    }
}

#endif //LEPTSERVER_CURRENTTHREAD_H
