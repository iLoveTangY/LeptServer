//
// Created by tang on 19-6-29.
//

#ifndef LEPTSERVER_EPOLL_H
#define LEPTSERVER_EPOLL_H

#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>
#include <noncopyable.h>

#include "Timer.h"
#include "Channel.h"


namespace lept_server
{
    class HttpData;

    class Epoll : lept_base::noncopyable
    {
    public:
        Epoll();
        ~Epoll();
        void epoll_add(ChannelPtr request, int timeout);
        void epoll_mod(ChannelPtr request, int timeout);
        void epoll_del(ChannelPtr request);
        std::vector<ChannelPtr> poll();

        void add_timer(ChannelPtr request, int timeout);
        void handle_expired()
        {
            timer_manager_.handle_expired();
        }

    private:
        std::vector<ChannelPtr> get_events_request(int events_num);

        static const int MAXFDS = 100000;
        int epoll_fd_;
        std::vector<epoll_event> events_;
        std::shared_ptr<Channel> fd_2_channel_[MAXFDS];
        std::shared_ptr<HttpData> fd_2_httpdata_[MAXFDS];
        TimerManager timer_manager_;
    };
}


#endif //LEPTSERVER_EPOLL_H
