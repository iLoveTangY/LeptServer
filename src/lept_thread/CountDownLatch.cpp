//
// Created by tang on 19-6-14.
//

#include "CountDownLatch.h"

lept_base::CountDownLatch::CountDownLatch(int count)
        : mutex_(), condition_(mutex_), count_(count)
{
}

void lept_base::CountDownLatch::wait()
{
    MutexLockGuard lock(mutex_);
    while (count_ > 0)
        condition_.wait();
}

void lept_base::CountDownLatch::count_down()
{
    MutexLockGuard lock(mutex_);
    --count_;
    if (count_ == 0)
        condition_.notify_all();
}
