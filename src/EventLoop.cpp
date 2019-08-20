#include <utility>

//
// Created by tang on 19-6-27.
//

#include <sys/eventfd.h>
#include <Logger.h>
#include <functional>
#include <unistd.h>

#include "EventLoop.h"
#include "Epoll.h"
#include "MutexLock.h"
#include "Utils.h"

__thread lept_server::EventLoop* loop_in_this_thread = nullptr;


int create_eventfd()
{
    int fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0)
    {
        LOG_ERROR << "Failed to create event fd\n";
        abort();
    }

    return fd;
}

lept_server::EventLoop::EventLoop()
        : looping_(false),
          poller_(new Epoll),
          wakeup_fd_(create_eventfd()),
          quit_(false),
          event_handling_(false),
          calling_pending_functors_(false),
          thread_id_(CurrentThreadInfo::tid()),
          p_wakeup_channel_(new Channel(this, wakeup_fd_))
{
    if (loop_in_this_thread != nullptr)
    {
        LOG_ERROR << "Another EventLoop in current thread\n";
        abort();
    }
    loop_in_this_thread = this;
    p_wakeup_channel_->set_events(EPOLLIN | EPOLLET);  // eventfd不用设置EPOLLONESHOT
    p_wakeup_channel_->set_read_handler(std::bind(&EventLoop::handle_read, this));
    p_wakeup_channel_->set_connection_handler(std::bind(&EventLoop::handle_connection, this));
    poller_->epoll_add(p_wakeup_channel_, 0);
}

lept_server::EventLoop::~EventLoop()
{
    close(wakeup_fd_);
    loop_in_this_thread = nullptr;
}

void lept_server::EventLoop::loop()
{
    assert(!looping_);
    assert(is_in_loopthread());
    looping_ = true;
    quit_ = false;

    LOG_INFO << "EventLoop " << this << " start looping\n";

    std::vector<ChannelPtr> ret;
    while (!quit_)
    {
        ret.clear();
        ret = poller_->poll();

        event_handling_ = true;
        for (auto &it : ret)
            it->handle_events();
        event_handling_ = false;

        do_pending_functors();
        poller_->handle_expired();
    }
    looping_ = false;
}

void lept_server::EventLoop::quit()
{
    quit_ = true;
    if (!is_in_loopthread())
    {
        wakeup();
    }
}

void lept_server::EventLoop::run_in_loop(lept_server::EventLoop::Functor &&cb)
{
    if (is_in_loopthread())
        cb();
    else
        queue_in_loop(std::move(cb));
}

void lept_server::EventLoop::queue_in_loop(lept_server::EventLoop::Functor &&cb)
{
    {
        // 主线程(MainReactor)和loop所在线程 (SubReactor) 会有锁争用
        lept_base::MutexLockGuard lock(mutex_);
        pending_functors_.emplace_back(std::move(cb));
    }
    if (!is_in_loopthread() || calling_pending_functors_)  // 第二个条件的原因？
        wakeup();
}

void lept_server::EventLoop::remove_from_poller(std::shared_ptr<lept_server::Channel> channel)
{
    poller_->epoll_del(std::move(channel));
}

void lept_server::EventLoop::update_poller(std::shared_ptr<lept_server::Channel> channel, int timeout)
{
    poller_->epoll_mod(std::move(channel), timeout);
}

void lept_server::EventLoop::add_to_poller(std::shared_ptr<lept_server::Channel> channel, int timeout)
{
    poller_->epoll_add(std::move(channel), timeout);
}

void lept_server::EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = writen(wakeup_fd_, reinterpret_cast<char *>(&one), sizeof(one));
    if (n != sizeof(one))
        LOG_DEBUG << "EventLoop::wakeup() writes " << n << " bytes instead of 8\n";
}

void lept_server::EventLoop::do_pending_functors()
{
    std::vector<Functor> functors;
    calling_pending_functors_ = true;

    {
        lept_base::MutexLockGuard lock(mutex_);
        functors.swap(pending_functors_);
    }

    for (auto &functor : functors)
        functor();
    calling_pending_functors_ = false;
}

void lept_server::EventLoop::handle_read()
{
    uint64_t one = 1;
    auto n = readn(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one))
        LOG_DEBUG << "EventLoop::handle_read() read " << n << "bytes instead of 8\n";
    p_wakeup_channel_->set_events(EPOLLIN | EPOLLET);
}

void lept_server::EventLoop::handle_connection()
{
    // 我觉得这里并不会有什么效果，因为前后两次注册的事件相同..
    // 由于wake_up_fd并没有采用EPOLLONESHOT，因此，这里的update_poller是不必要的
    // TODO 这里可以把update_poller删掉？
    update_poller(p_wakeup_channel_, 0);
}
