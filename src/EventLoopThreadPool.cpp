//
// Created by tang on 19-7-2.
//

#include <cassert>

#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

void lept_server::EventLoopThreadPool::start()
{
    base_loop_->assert_in_loopthread();
    started_ = true;
    for (int i = 0; i < num_threads_; ++i)
    {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());
        threads_.push_back(t);
        loops_.push_back(t->start_loop());
    }
}

lept_server::EventLoopThreadPool::EventLoopThreadPool(lept_server::EventLoop *base_loop, int num_threads)
        : base_loop_(base_loop),
          started_(false),
          num_threads_(num_threads),
          next_(0)
{
    assert(num_threads >= 0);
}

lept_server::EventLoop *lept_server::EventLoopThreadPool::get_next()
{
    base_loop_->assert_in_loopthread();
    assert(started_);
    EventLoop *loop = base_loop_;
    if (!loops_.empty())
    {
        // Round Robin 分配线程
        loop = loops_[next_];
        next_ = (next_ + 1) % num_threads_;
    }
    return loop;
}
