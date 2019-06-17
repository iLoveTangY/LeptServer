//
// Created by tang on 19-6-16.
//
#include "CurrentThreadInfo.h"

#include <cstdio>  // snprintf
#include <syscall.h>  // SYS_gettid
#include <unistd.h>  // ::syscall

namespace CurrentThreadInfo
{
    __thread int cached_tid = 0;  // pid_t形式的线程id
    __thread char tid_string[32];  // 字符串形式的线程id（__thread）无法修饰class类型，只能用char[]
    __thread const char *thread_name = "LeptThread";  // 线程名称

    pid_t get_tid()
    {
        return static_cast<pid_t>(::syscall(SYS_gettid));
    }

    void cache_tid()
    {
        if (cached_tid == 0)
        {
            cached_tid = get_tid();
            snprintf(tid_string, sizeof(tid_string), "%5d ", cached_tid);
        }
    }
}

