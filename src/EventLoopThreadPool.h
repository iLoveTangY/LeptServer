//
// Created by tang on 19-7-2.
//

#ifndef LEPTSERVER_EVENTLOOPTHREADPOOL_H
#define LEPTSERVER_EVENTLOOPTHREADPOOL_H

#include <vector>
#include <memory>

#include <noncopyable.h>
#include <Logger.h>

namespace lept_server
{
    class EventLoop;
    class EventLoopThread;

    class EventLoopThreadPool : lept_base::noncopyable
    {
    public:
        EventLoopThreadPool(EventLoop* base_loop, int num_threads);
        ~EventLoopThreadPool()
        {
            LOG_DEBUG << "~EventLoopThreadPool()\n";
        }

        void start();

        EventLoop *get_next();

    private:
        EventLoop *base_loop_;
        bool started_;
        int num_threads_;
        int next_;
        std::vector<std::shared_ptr<EventLoopThread>> threads_;
        std::vector<EventLoop*> loops_;
    };
}


#endif //LEPTSERVER_EVENTLOOPTHREADPOOL_H
