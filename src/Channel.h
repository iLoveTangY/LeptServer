//
// Created by tang on 19-6-28.
//

#ifndef LEPTSERVER_CHANNEL_H
#define LEPTSERVER_CHANNEL_H


#include <noncopyable.h>
#include <functional>
#include <memory>
#include <sys/epoll.h>

namespace lept_server
{
    class EventLoop;
    class HttpData;

    class Channel : lept_base::noncopyable
    {
    public:
        using CallBack = std::function<void()>;
        using EventType = __uint32_t ;

        // outline ctor and dtor
        explicit Channel(EventLoop *loop);
        Channel(EventLoop *loop, int fd);
        ~Channel();
        int get_fd()
        {
            return fd_;
        }
        void set_fd(int fd)
        {
            fd_ = fd;
        }

        void set_holder(const std::shared_ptr<HttpData> &holder)
        {
            holder_ = holder;
        }

        std::shared_ptr<HttpData> get_holder()
        {
            std::shared_ptr<HttpData> ret(holder_.lock());
            return ret;
        }

        void set_read_handler(CallBack &&read_handler)
        {
            read_handler_ = read_handler;
        }

        void set_write_handler(CallBack &&write_handler)
        {
            write_handler_ = write_handler;
        }

        void set_error_handler(CallBack &&error_handler)
        {
            error_handler_ = error_handler;
        }

        void set_connection_handler(CallBack &&connection_handler)
        {
            connection_handler_ = connection_handler;
        }

        void handle_events()
        {
            events_ = 0;
            if ((r_events_ & EPOLLHUP) && !(r_events_ & EPOLLIN))
            {
                // 对端主动断开连接
                events_ = 0;
                return;
            }
            if (r_events_ & EPOLLERR)
            {
                // 服务端出错
                if (error_handler_)
                    error_handler_();
                events_ = 0;
                return;
            }
            if (r_events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
            {
                // 有数据可读
                if (read_handler_)
                    read_handler_();
            }
            if (r_events_ & EPOLLOUT)
            {
                // 可写
                if (write_handler_)
                    write_handler_();
            }

            if (connection_handler_)
                connection_handler_();
        }

        void set_r_events(EventType ev)
        {
            r_events_ = ev;
        }

        void set_events(EventType ev)
        {
            events_ = ev;
        }

        EventType get_events()
        {
            return events_;
        }

        EventType get_last_events()
        {
            return last_events_;
        }

        bool equal_and_update_last_events()
        {
            bool ret = (last_events_ == events_);
            last_events_ = events_;
            return ret;
        }

    private:
        EventLoop *loop_;  // Channel所属事件循环
        int fd_;    // Channel并不持有此fd，HttpData中才持有此fd

        EventType events_;  // 关心的事件
        EventType r_events_;  // 当前Channel的活动事件
        EventType last_events_;  // 当前Channel上次所关心的事件

        std::weak_ptr<HttpData> holder_;  // 持有此Channel的HttpData対像,只对于Accepted fd才有此对象，Listenfd不会交由HttpData来管理

        CallBack read_handler_;
        CallBack write_handler_;
        CallBack error_handler_;
        CallBack connection_handler_;
    };

    using ChannelPtr = std::shared_ptr<Channel>;
}


#endif //LEPTSERVER_CHANNEL_H
