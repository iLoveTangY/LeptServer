//
// Created by tang on 19-6-14.
//

#ifndef LEPTSERVER_MUTEXLOCK_H
#define LEPTSERVER_MUTEXLOCK_H

#include "noncopyable.h"
#include <pthread.h>

namespace lept_server
{
    /**
     * 不应该直接使用这个类的加锁解锁方法，而应该使用下面的MutextLockGurad类
     * 禁止拷贝
     */
    class MutexLock : noncopyable
    {
    public:
        MutexLock()
        {
            // TODO 有必要设置互斥量属性为normal而不是使用default
            // TODO 没有进行错误检查
            pthread_mutex_init(&mutex_, nullptr);
        }

        ~MutexLock()
        {
            pthread_mutex_destroy(&mutex_);
        }

        // 加锁，禁止用户代码调用，仅供MutexLockGuard调用
        void lock()
        {
            pthread_mutex_lock(&mutex_);
        }

        // 解锁，同样禁止用户代码调用
        void unlock()
        {
            pthread_mutex_unlock(&mutex_);
        }

        // 同样禁止用户代码调用
        pthread_mutex_t *get_mutex()
        {
            return &mutex_;
        }

    private:
        pthread_mutex_t mutex_{};

        friend class Condition;
    };

    /**
     * RAII手法的封装，禁止拷贝
     */
    class MutexLockGuard : noncopyable
    {
    private:
        MutexLock &mutex_;
    public:
        explicit MutexLockGuard(MutexLock &mutex) : mutex_(mutex)
        {
            mutex_.lock();
        }

        // 析构时自动解锁
        ~MutexLockGuard()
        {
            mutex_.unlock();
        }
    };

// 防止出现未使用变量名定义一个临时变量，加锁又马上解锁的情况发生
#define MutexLockGuard(x) static_assert(false, "missing mutex gurad var name")
}


#endif //LEPTSERVER_MUTEXLOCK_H
