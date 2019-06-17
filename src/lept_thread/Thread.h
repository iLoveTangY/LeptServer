//
// Created by tang on 19-6-14.
//

#ifndef LEPTSERVER_THREAD_H
#define LEPTSERVER_THREAD_H

#include "noncopyable.h"
#include "CountDownLatch.h"
#include <functional>
#include <memory.h>
#include <pthread.h>
#include <string>
#include <sys/syscall.h>
#include <unistd.h>

namespace lept_server
{
    /**
     * 线程类的封装，会创建一个新的线程来执行指定的函数
     */
    class Thread : noncopyable
    {
    public:
        using ThreadFunc = std::function<void()>;

        explicit Thread(const ThreadFunc&, const std::string &name = std::string());

        ~Thread();

        void start();

        int join();

        bool started() const
        {
            return started_;
        }

        pid_t tid() const
        {
            return tid_;
        }

        const std::string& name() const
        {
            return name_;
        }

    private:
        void set_default_name();

        bool started_;  // 新创建的线程是否已经启动
        bool joined_;  // 新创建的线程是否已经join
        pthread_t pthread_id_;  // 启动的新的线程id（pthread_t类型）
        pid_t tid_;  // 用户可见的线程id（pid_t类型）
        std::string name_;  // 新创建的线程的名称
        ThreadFunc func_;  // 要执行的函数
        CountDownLatch latch_;
    };
}

#endif //LEPTSERVER_THREAD_H
