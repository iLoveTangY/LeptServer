//
// Created by tang on 19-6-16.
//

#include "Thread.h"
#include "CurrentThreadInfo.h"
#include <cassert>
#include <sys/prctl.h>

using namespace std;
using namespace lept_base;

struct ThreadData
{
    using ThreadFunc = Thread::ThreadFunc;
    const ThreadFunc &func_;  // 这里可以传引用，不用担心外界改变函数，因为Thread中压根没有提供修改func的方法
    string name_;  // 其实这里也可以使用引用类型
    pid_t &tid_;
    CountDownLatch &latch_;

    ThreadData(const ThreadFunc &func, string name, pid_t &tid, CountDownLatch &latch)
            : func_(func), name_(std::move(name)), tid_(tid), latch_(latch)
    {}

    // 执行到run函数时已经在一个新的线程中了
    void run()
    {
        tid_ = CurrentThreadInfo::tid();
        latch_.count_down();

        CurrentThreadInfo::thread_name = name_.c_str();
        prctl(PR_SET_NAME, CurrentThreadInfo::get_thread_name());

        // 执行指定的函数
        func_();

        CurrentThreadInfo::thread_name = "Finished";
        prctl(PR_SET_NAME, CurrentThreadInfo::get_thread_name());
    }
};

// 传递给pthread_create的接口函数
void *start(void* obj)
{
    auto data = static_cast<ThreadData*>(obj);
    data->run();
    delete data;
    return nullptr;
}


lept_base::Thread::Thread(const lept_base::Thread::ThreadFunc &func, const std::string &name)
        : started_(false), joined_(false), pthread_id_(0), tid_(0), name_(name), func_(func), latch_(1)
{
    set_default_name();
}

void lept_base::Thread::set_default_name()
{
    if (name_.empty())
        name_ = "LeptThread";
}

lept_base::Thread::~Thread()
{
    if (started_ && !joined_)
        pthread_detach(pthread_id_);
}

void lept_base::Thread::start()
{
    assert(!started_);
    started_ = true;
    auto data = new ThreadData(func_, name_, tid_, latch_);
    if (pthread_create(&pthread_id_, nullptr, &::start, data))
    {
        started_ = false;
        delete data;
    }
    else
    {
        latch_.wait();
        assert(tid_ > 0);
    }
}

// 等待创建的线程终止
int lept_base::Thread::join()
{
    assert(started_);
    assert(!joined_);
    joined_ = true;
    return pthread_join(pthread_id_, nullptr);
}
