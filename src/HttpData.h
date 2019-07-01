//
// Created by tang on 19-6-27.
//

#ifndef LEPTSERVER_HTTPDATA_H
#define LEPTSERVER_HTTPDATA_H

#include <memory>
#include <unordered_map>
#include <unistd.h>

namespace lept_server
{
    class EventLoop;
    class Channel;
    class TimerNode;

    class HttpData : std::enable_shared_from_this<HttpData>
    {
    public:
        HttpData(EventLoop *loop, int connfd);
        ~HttpData()
        {
            close(fd_);
        }

        void reset();
        void seprate_timer();
        void link_timer(const std::shared_ptr<TimerNode> &timer)
        {
            timer_ = timer;
        }
        std::shared_ptr<Channel> get_channel()
        {
            return channel_;
        }
        void handle_close();
        void new_event();

    private:
        EventLoop *loop_;
        std::shared_ptr<Channel> channel_;
        int fd_;
        std::string in_buffer_;
        std::string out_buffer_;
        bool error_;

        bool keep_alive_;
        std::unordered_map<std::string, std::string> headers_;
        std::weak_ptr<TimerNode> timer_;

        void handle_read();
        void handle_write();
        void handle_connection();
        void handle_error(int fd, int err_num, std::string short_message);

    };
}


#endif //LEPTSERVER_HTTPDATA_H
