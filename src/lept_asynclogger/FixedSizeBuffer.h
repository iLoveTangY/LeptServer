//
// Created by tang on 19-6-17.
//

#ifndef LEPTSERVER_FIXEDSIZEBUFFER_H
#define LEPTSERVER_FIXEDSIZEBUFFER_H

#include <cstdio>
#include <cstring>
#include "noncopyable.h"

namespace lept_base
{
    template<int SIZE>
    class FixedSizeBuffer : noncopyable
    {
    public:
        FixedSizeBuffer() : cur_(data_)
        {
        }

        size_t append(const char* buf, size_t len)
        {
            if (avail_size() > static_cast<int>(len))
            {
                memcpy(cur_, buf, len);
                cur_ += len;
                return len;
            }
            return 0;
        }

        inline const char *data() const
        {
            return data_;
        }

        // 已使用的缓冲区长度
        inline size_t length() const
        {
            return static_cast<size_t>(cur_ - data_);
        }

        inline char *current()
        {
            return cur_;
        }

        // 目前缓冲区可用大小
        inline int avail_size() const
        {
            return static_cast<int>(end() - cur_);
        }

        inline void add(size_t len)
        {
            cur_ += len;
        }

        /**
         * 重新从头开始使用缓冲区
         */
        inline void reset()
        {
            cur_ = data_;
        }

        /**
         * 将缓冲区所有元素置为0
         * 置为0之后不需要设置cur的位置？
         */
        inline void bzero()
        {
            memset(data_, 0, sizeof(data_));
        }

    private:
        /**
         * 缓冲区末尾下一个位置
         */
        inline const char *end() const
        {
            return data_ + sizeof(data_);
        }

        char data_[SIZE]{};  // 真正的数据存储位置
        char *cur_;  // 缓冲区可用位置指针
    };
}

const int SmallBuffer = 4000;
const int LargerBuffer = 4000 * 1000;

#endif //LEPTSERVER_FIXEDSIZEBUFFER_H
