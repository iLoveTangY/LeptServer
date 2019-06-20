//
// Created by tang on 19-6-18.
//

#ifndef LEPTSERVER_ASYNCLOGGER_H
#define LEPTSERVER_ASYNCLOGGER_H

#include <vector>
#include <memory>
#include <cassert>
#include "noncopyable.h"
#include "FixedSizeBuffer.h"
#include "Thread.h"

namespace lept_base
{
    class AsyncLogger : noncopyable
    {
    public:
        explicit AsyncLogger(const std::string &filename, int flush_interval = 3);

        void append(const char *logline, size_t len);

        ~AsyncLogger()
        {
            if (running_)
                stop();
        }

        inline void start()
        {
            running_ = true;
            thread_.start();
            latch_.wait();
        }

        inline void stop()
        {
            assert(running_);
            running_ = false;
            cond_.notify();
            thread_.join();
        }

    private:
        void thread_func();

        using Buffer = FixedSizeBuffer<LargerBuffer>;
        using BufferVector = std::vector<std::shared_ptr<Buffer>>;
        using BufferPtr = std::shared_ptr<Buffer>;

        const int flush_interval_;
        bool running_;
        std::string file_name_;
        Thread thread_;
        MutexLock mutex_;
        Condition cond_;
        BufferPtr current_buffer_ptr_;
        BufferPtr next_buffer_ptr_;
        BufferVector buffer_ptrs_;
        CountDownLatch latch_;
    };
}


#endif //LEPTSERVER_ASYNCLOGGER_H
