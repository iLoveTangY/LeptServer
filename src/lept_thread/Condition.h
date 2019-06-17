//
// Created by tang on 19-6-14.
//

#ifndef LEPTSERVER_CONDITION_H
#define LEPTSERVER_CONDITION_H

#include <pthread.h>
#include <ctime>
#include <cerrno>
#include "MutexLock.h"
#include "noncopyable.h"

namespace lept_server
{
    /**
     * 条件变量的RAII封装
     */
    class Condition : noncopyable
    {
    public:
        explicit Condition(MutexLock &mutex) : mutex_(mutex)
        {
            // TODO 检查返回值
            pthread_cond_init(&cond_, nullptr);
        }

        ~Condition()
        {
            pthread_cond_destroy(&cond_);
        }

        void wait()
        {
            pthread_cond_wait(&cond_, mutex_.get_mutex());
        }

        void notify()
        {
            pthread_cond_signal(&cond_);
        }

        void notify_all()
        {
            pthread_cond_broadcast(&cond_);
        }

        // 等待给定时间(秒)
        bool wait_for_seconds(int seconds)
        {
            timespec abstime;
            clock_gettime(CLOCK_REALTIME, &abstime);
            abstime.tv_sec += static_cast<time_t>(seconds);
            return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.get_mutex(), &abstime);
        }

    private:
        MutexLock &mutex_;  // 只能使用引用类型，因为禁止拷贝
        pthread_cond_t cond_;
    };
}

#endif //LEPTSERVER_CONDITION_H
