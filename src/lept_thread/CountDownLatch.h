//
// Created by tang on 19-6-14.
//

#ifndef LEPTSERVER_COUNTDOWNLATCH_H
#define LEPTSERVER_COUNTDOWNLATCH_H

#include "noncopyable.h"
#include "Condition.h"
#include "MutexLock.h"

namespace lept_base
{
    /**
     * 倒计时
     */
    class CountDownLatch : noncopyable
    {
    public:
        explicit CountDownLatch(int count);
        void wait();
        void count_down();

    private:
        MutexLock mutex_;
        Condition condition_;
        int count_;
    };
}


#endif //LEPTSERVER_COUNTDOWNLATCH_H
