//
// Created by tang on 19-7-3.
//

#include <Utils.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Server.h"
#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "Channel.h"
#include "HttpData.h"

lept_server::Server::Server(lept_server::EventLoop *loop, int thread_num, int port)
        : loop_(loop),
          thread_num_(thread_num),
          thread_pool_(new EventLoopThreadPool(loop_, thread_num)),
           started_(false),
           accept_channel_(new Channel(loop_)),
           port_(port),
           listen_fd_(socket_bind_listen(port))
{
    accept_channel_->set_fd(listen_fd_);
    handle_for_sigpipe();
    if (setSocketNonBlocking(listen_fd_) < 0)
    {
        perror("set socket non block failed");
        LOG_ERROR << "Set Socket non block failed\n";
        abort();
    }
}

void lept_server::Server::start()
{
    thread_pool_->start();
    accept_channel_->set_events(EPOLLIN | EPOLLET);
    accept_channel_->set_read_handler(std::bind(&Server::handle_new_connection, this));
    accept_channel_->set_connection_handler(std::bind(&Server::handle_this_connection, this));
    loop_->add_to_poller(accept_channel_, 0);
    started_ = true;
    loop_->loop();
}

void lept_server::Server::handle_new_connection()
{
    sockaddr_in client_addr{};
    memset(&client_addr, 0, sizeof(sockaddr_in));
    socklen_t client_addr_len = sizeof(client_addr);
    int accept_fd = 0;

    while ((accept_fd = accept(listen_fd_, (sockaddr *)&client_addr, &client_addr_len)) > 0)
    {
        auto loop = thread_pool_->get_next();
        LOG_INFO << "New Connection from " << inet_ntoa(client_addr.sin_addr) << " : " << ntohs(client_addr.sin_port);
        if (accept_fd >= MAXFDS)
        {
            close(accept_fd);
            continue;
        }

        if (setSocketNonBlocking(accept_fd) < 0)
        {
            LOG_DEBUG << "Set socket non block failed\n";
            return;
        }

        setSocketNodelay(accept_fd);

        std::shared_ptr<HttpData> request(new HttpData(loop, accept_fd));
        request->get_channel()->set_holder(request);
        loop->queue_in_loop(std::bind(&HttpData::new_event, request));
    }
    accept_channel_->set_events(EPOLLIN | EPOLLET);
}

void lept_server::Server::handle_this_connection()
{
    loop_->update_poller(accept_channel_);
}
