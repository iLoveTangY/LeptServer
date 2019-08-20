//
// Created by tang on 19-6-27.
//

#ifndef LEPTSERVER_TIMER_H
#define LEPTSERVER_TIMER_H

#include <cstdio>
#include <memory>
#include <queue>

namespace lept_server
{
    class HttpData;

    class TimerNode
    {
    public:
        TimerNode(std::shared_ptr<HttpData> http_data, int timeout);
        ~TimerNode();
        TimerNode(TimerNode &tn);
        bool is_valid();  // 当前TimerNode节点是否有效
        void clear_data();
        void set_deleted()
        {
            deleted_ = true;
        }
        bool is_deleted() const
        {
            return deleted_;
        }
        time_t get_expire_time() const
        {
            return expired_time_;
        }

    private:
        bool deleted_;  // 是否已经被删除
        time_t expired_time_;  // 超时时间
        std::shared_ptr<HttpData> owner_http_data_;
    };

    struct TimerNodeCmp
    {
        bool operator()(const std::shared_ptr<TimerNode> &lhs, const std::shared_ptr<TimerNode> &rhs)
        {
            return lhs->get_expire_time() > rhs->get_expire_time();
        }
    };

    class TimerManager
    {
    public:
        TimerManager() = default;
        ~TimerManager() = default;
        void addTimer(std::shared_ptr<HttpData> http_data, int timeout);
        void handle_expired();

    private:
        using SPTimerNode = std::shared_ptr<TimerNode>;
        std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerNodeCmp> timer_node_heap_;
    };
}


#endif //LEPTSERVER_TIMER_H
