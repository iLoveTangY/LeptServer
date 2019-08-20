//
// Created by tang on 19-7-2.
//

#ifndef LEPTSERVER_EVENTLOOPTHREAD_H
#define LEPTSERVER_EVENTLOOPTHREAD_H

#include "Thread.h"

#include "noncopyable.h"

namespace lept_server
{
    class EventLoop;

    class EventLoopThread : lept_base::noncopyable
    {
    public:
        EventLoopThread()
                : loop_(nullptr),
                  exiting_(false),
                  thread_(std::bind(&EventLoopThread::thread_func, this), "EventLoopThread"),
                  mutex_(),
                  cond_(mutex_)
        {
        }

        ~EventLoopThread();

        EventLoop *start_loop();

    private:
        void thread_func();

        EventLoop *loop_;  // 考虑采用shared_ptr?
        bool exiting_;
        lept_base::Thread thread_;
        lept_base::MutexLock mutex_;
        lept_base::Condition cond_;
    };
}


#endif //LEPTSERVER_EVENTLOOPTHREAD_H
