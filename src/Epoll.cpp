//
// Created by tang on 19-6-29.
//

#include <cassert>
#include "Epoll.h"
#include "HttpData.h"
#include "Logger.h"

const int EVENTSUM = 4096;
const int EPOLLWAIT_TIME = 10000;

lept_server::Epoll::Epoll()
        : epoll_fd_(epoll_create1(EPOLL_CLOEXEC)),
        events_(EVENTSUM)
{
    assert(epoll_fd_ > 0);
}

lept_server::Epoll::~Epoll() = default;

void lept_server::Epoll::epoll_add(lept_server::ChannelPtr request, int timeout)
{
    int fd = request->get_fd();
    if (timeout > 0)
    {
        add_timer(request, timeout);
        fd_2_httpdata_[fd] = request->get_holder();  // 为什么是在这？因为有可能有Listenfd会添加进来，而只有timeout大于0的才是Acceptfd，这里写的不好
    }
    epoll_event event{};
    event.data.fd = fd;
    event.events = request->get_events();

    request->equal_and_update_last_events();  // 更新下last_events，不管last_events和events等不等都得添加

    fd_2_channel_[fd] = request;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        LOG_DEBUG << "epoll add error: " << strerror(errno) << "\n";
        fd_2_channel_[fd].reset();  // 要释放掉当前拷贝的智能指针
    }
}

void lept_server::Epoll::epoll_mod(lept_server::ChannelPtr request, int timeout)
{
    if (timeout > 0)
        add_timer(request, timeout);
    int fd = request->get_fd();
    if (!request->equal_and_update_last_events())  // 如果上次注册的时间和当前的事件相同，则不需要重新在epoll中注册了
    {
        epoll_event event{};
        event.data.fd = fd;
        event.events = request->get_events();
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) < 0)
        {
            LOG_DEBUG << "epoll mod error: " << strerror(errno) << "\n";
            fd_2_channel_[fd].reset();
        }
    }
}

void lept_server::Epoll::epoll_del(lept_server::ChannelPtr request)
{
    int fd = request->get_fd();
    epoll_event event{};
    event.data.fd = fd;
    event.events = request->get_last_events();  // 这里其实last_events和events都是一样的

    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        LOG_DEBUG << "epoll del error: " << strerror(errno) << "\n";
    }
    fd_2_channel_[fd].reset();
    fd_2_httpdata_[fd].reset();
}

std::vector<lept_server::ChannelPtr> lept_server::Epoll::poll()
{
    while (true)
    {
        // TODO 等待时间可以设置为TimerManager中最小的超时时间
        int event_count = epoll_wait(epoll_fd_, &*events_.begin(), static_cast<int>(events_.size()), EPOLLWAIT_TIME);
        if (event_count < 0)
            LOG_DEBUG << "epoll wait error: " << strerror(errno) << "\n";
        std::vector<ChannelPtr> req_data = get_events_request(event_count);
        if (!req_data.empty())
            return req_data;
    }
}

void lept_server::Epoll::add_timer(lept_server::ChannelPtr request, int timeout)
{
    std::shared_ptr<HttpData> t = request->get_holder();
    if (t)
        timer_manager_.addTimer(t, timeout);
    else
        LOG_DEBUG << "Invalid request data\n";
}

std::vector<lept_server::ChannelPtr> lept_server::Epoll::get_events_request(int events_num)
{
    std::vector<ChannelPtr> req_data;
    for (int i = 0; i < events_num; ++i)
    {
        int fd = events_[i].data.fd;

        ChannelPtr cur_req = fd_2_channel_[fd];

        if (cur_req)
        {
            cur_req->set_r_events(events_[i].events);
            cur_req->set_events(0);  // 设置为0，需要客户端重新设置
            req_data.push_back(cur_req);
        }
        else
        {
            LOG_DEBUG << "cur_req is invalid\n";
        }
    }
    return req_data;
}
