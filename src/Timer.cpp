#include <utility>
#include <sys/time.h>

//
// Created by tang on 19-6-27.
//

#include "Timer.h"
#include "HttpData"

lept_server::TimerNode::TimerNode(std::shared_ptr<lept_server::HttpData> http_data, int timeout)
        : deleted_(false), owner_http_data_(std::move(http_data))
{
    timeval now{};
    gettimeofday(&now, nullptr);
    // 超时时间，以毫秒来计算
    expired_time_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}
