//
// Created by tang on 19-7-2.
//

#include "EventLoopThread.h"
#include "EventLoop.h"

lept_server::EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr)
    {
        loop_->quit();
        thread_.join();
    }
}

lept_server::EventLoop *lept_server::EventLoopThread::start_loop()
{
    assert(!thread_.started());
    thread_.start();

    {
        lept_base::MutexLockGuard lock(mutex_);
        while (loop_ == nullptr)
            cond_.wait();
    }

    return loop_;
}

void lept_server::EventLoopThread::thread_func()
{
    EventLoop loop;

    {
        lept_base::MutexLockGuard lock(mutex_);
        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();
    loop_ = nullptr;
}
