//
// Created by tang on 19-7-3.
//

#ifndef LEPTSERVER_SERVER_H
#define LEPTSERVER_SERVER_H


#include <noncopyable.h>
#include <memory>
#include "EventLoopThreadPool.h"

namespace lept_server
{
    class EventLoop;
    class EventLoopThreadPool;
    class Channel;

    class Server : lept_base::noncopyable
    {
    public:
        Server(EventLoop *loop, int thread_num, int port);

        void start();
        void handle_new_connection();
        inline void handle_this_connection();

    private:
        // 主事件循环
        EventLoop *loop_;
        // 线程数量
        int thread_num_;

        std::unique_ptr<EventLoopThreadPool> thread_pool_;
        bool started_;
        std::shared_ptr<Channel> accept_channel_;
        int port_;
        int listen_fd_;
        static const int MAXFDS = 100000;
    };
}


#endif //LEPTSERVER_SERVER_H
