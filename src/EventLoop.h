//
// Created by tang on 19-6-27.
//

#ifndef LEPTSERVER_EVENTLOOP_H
#define LEPTSERVER_EVENTLOOP_H

#include <memory>
#include <functional>
#include <vector>
#include <MutexLock.h>
#include <cassert>

#include "noncopyable.h"
#include "CurrentThreadInfo.h"

namespace lept_server
{
    class Channel;
    class Epoll;

    class EventLoop : lept_base::noncopyable
    {
    public:
        using Functor = std::function<void()>;

        EventLoop();
        ~EventLoop();
        void loop();
        void quit();
        void run_in_loop(Functor &&cb);
        void queue_in_loop(Functor &&cb);
        bool is_in_loopthread() const
        {
            return thread_id_ == CurrentThreadInfo::tid();
        };

        void assert_in_loopthread()
        {
            assert(is_in_loopthread());
        }

        inline void remove_from_poller(std::shared_ptr<Channel> channel);

        inline void update_poller(std::shared_ptr<Channel> channel, int timeout = 0);

        inline void add_to_poller(std::shared_ptr<Channel> channel, int timeout = 0);

    private:
        void wakeup();
        void do_pending_functors();
        // handle_read和handle_connection是wake_up_fd的处理函数
        void handle_read();
        void handle_connection();

        bool looping_;
        std::shared_ptr<Epoll> poller_;
        int wakeup_fd_;  // 一个eventfd，用来唤醒当前EventLoop所属事件循环
        bool quit_;
        bool event_handling_;
        lept_base::MutexLock mutex_; // 保护pending_functors_
        std::vector<Functor> pending_functors_;  // 等待在本事件循环中执行的函数
        bool calling_pending_functors_;
        const pid_t thread_id_;  // 事件循环所属线程
        std::shared_ptr<Channel> p_wakeup_channel_;
    };
}


#endif //LEPTSERVER_EVENTLOOP_H
