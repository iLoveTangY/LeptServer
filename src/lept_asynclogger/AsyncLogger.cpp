//
// Created by tang on 19-6-18.
//

#include <cassert>
#include "AsyncLogger.h"
#include "LogFile.h"

lept_base::AsyncLogger::AsyncLogger(const std::string &filename, int flush_interval)
        : flush_interval_(flush_interval),
          running_(false),
          file_name_(filename),
          thread_(std::bind(&AsyncLogger::thread_func, this), "Logger"),
          mutex_(),
          cond_(mutex_),
          current_buffer_ptr_(new Buffer),
          next_buffer_ptr_(new Buffer),
          buffer_ptrs_(),
          latch_(1)
{
    current_buffer_ptr_->bzero();
    next_buffer_ptr_->bzero();
    buffer_ptrs_.reserve(16);
}

void lept_base::AsyncLogger::append(const char *logline, size_t len)
{
    MutexLockGuard lock(mutex_);
    if (current_buffer_ptr_->avail_size() > len)
        current_buffer_ptr_->append(logline, len);
    else
    {
        buffer_ptrs_.push_back(current_buffer_ptr_);
        current_buffer_ptr_.reset();
        if (next_buffer_ptr_)  // 如果准备的第二个缓冲区还未使用则现在可以使用了
            current_buffer_ptr_ = std::move(next_buffer_ptr_);  // move之后next_buffer_ptr为nullptr
        else  // 如果已经使用那么就要新开辟一个缓冲区了
            current_buffer_ptr_.reset(new Buffer);
        current_buffer_ptr_->append(logline, len);
        cond_.notify();  // 通知后段进程可以开始写入文件了
    }
}

/**
 * 后端线程要执行的写入到文件的程序
 */
void lept_base::AsyncLogger::thread_func()
{
    assert(running_);
    latch_.count_down();
    LogFile output(file_name_);
    // 为了减小临界区的大小，我们定义一些栈上的变量来使用
    BufferPtr new_buffer_1_ptr(new Buffer);
    BufferPtr new_buffer_2_ptr(new Buffer);
    new_buffer_1_ptr->bzero();
    new_buffer_2_ptr->bzero();
    BufferVector buffer_2_write_ptrs_;
    buffer_2_write_ptrs_.reserve(16);

    while (running_)
    {
        assert(new_buffer_1_ptr && new_buffer_1_ptr->length() == 0);
        assert(new_buffer_2_ptr && new_buffer_2_ptr->length() == 0);
        assert(buffer_2_write_ptrs_.empty());

        {
            MutexLockGuard lock(mutex_);
            if (buffer_ptrs_.empty())
                cond_.wait_for_seconds(flush_interval_);  // 不管有没有缓冲区被写满，最多等待flush_interval秒就要写入一次数据到文件

            buffer_ptrs_.push_back(current_buffer_ptr_);
            current_buffer_ptr_.reset();

            current_buffer_ptr_ = std::move(new_buffer_1_ptr);  // 重新设置好前端线程所要使用的第一个缓冲区
            buffer_2_write_ptrs_.swap(buffer_ptrs_);  // swap是内部指针交换，而非复制，所以代价很低
            if (!next_buffer_ptr_)  // 如果前端线程的下一个缓冲区也使用了，那么也需要重新设置
                next_buffer_ptr_ = std::move(new_buffer_2_ptr);
        }

        assert(!buffer_2_write_ptrs_.empty());

        if (buffer_2_write_ptrs_.size() > 25)  // 如果太多的话我们丢弃掉一些，只保留两个
            buffer_2_write_ptrs_.erase(buffer_2_write_ptrs_.begin() + 2, buffer_2_write_ptrs_.end());

        for (auto &b : buffer_2_write_ptrs_)
            output.append(b->data(), b->length());  // 这里真正写入到文件

        if (buffer_2_write_ptrs_.size() > 2)
            buffer_2_write_ptrs_.resize(2);

        // 以下重新设置好两个本地变量以供下次循环时使用
        if (!new_buffer_1_ptr)
        {
            assert(!buffer_2_write_ptrs_.empty());
            new_buffer_1_ptr = buffer_2_write_ptrs_.back();
            buffer_2_write_ptrs_.pop_back();
            new_buffer_1_ptr->reset();
        }

        if (!new_buffer_2_ptr)
        {
            assert(!buffer_2_write_ptrs_.empty());
            new_buffer_2_ptr = buffer_2_write_ptrs_.back();
            buffer_2_write_ptrs_.pop_back();
            new_buffer_2_ptr->reset();
        }
        buffer_2_write_ptrs_.clear();
        output.flush();
    }
    output.flush();
}
