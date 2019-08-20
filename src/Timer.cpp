#include <utility>

#include <utility>
#include <sys/time.h>

//
// Created by tang on 19-6-27.
//

#include "Timer.h"
#include "HttpData.h"

lept_server::TimerNode::TimerNode(std::shared_ptr<lept_server::HttpData> http_data, int timeout)
        : deleted_(false), owner_http_data_(std::move(http_data))
{
    timeval now{};
    gettimeofday(&now, nullptr);
    // 超时时间，以毫秒来计算
    expired_time_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

lept_server::TimerNode::~TimerNode()
{
    if (owner_http_data_)
        owner_http_data_->handle_close();
}

lept_server::TimerNode::TimerNode(lept_server::TimerNode &tn)
        : owner_http_data_(tn.owner_http_data_),
          deleted_(tn.deleted_),
          expired_time_(tn.expired_time_)
{
}

bool lept_server::TimerNode::is_valid()
{
    timeval now{};
    gettimeofday(&now, nullptr);
    auto temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
    if (temp < expired_time_)  // 当前时间小于超时时间，未超时
        return true;
    else
    {
        this->set_deleted();  // lazy delete
        return false;
    }
}

void lept_server::TimerNode::clear_data()
{
    owner_http_data_.reset();
    this->set_deleted();
}

void lept_server::TimerManager::addTimer(std::shared_ptr<lept_server::HttpData> http_data, int timeout)
{
    std::shared_ptr<TimerNode> new_node(new TimerNode(http_data, timeout));
    timer_node_heap_.push(new_node);
    http_data->link_timer(new_node);
}

void lept_server::TimerManager::handle_expired()
{
    while (!timer_node_heap_.empty())
    {
        auto ptimer = timer_node_heap_.top();
        // 一个节点直到前面的节点都被删除了才会真正被删除
        if (ptimer->is_deleted())
            timer_node_heap_.pop();
        else if (!ptimer->is_valid())
            timer_node_heap_.pop();
        else
            break;
    }
}
