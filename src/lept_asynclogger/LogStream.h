//
// Created by tang on 19-6-17.
//

#ifndef LEPTSERVER_LOGSTREAM_H
#define LEPTSERVER_LOGSTREAM_H

#include "noncopyable.h"
#include "FixedSizeBuffer.h"
#include <string>
#include <algorithm>
#include <iostream>

extern const char digits[];  // 为了负数时也通用定义成这个样子
extern const char *zero;

/**
 * 整数转字符串
 * 注意，这里的T只能是整数类型，包括int，short，unsigned int...等
 */
template<typename T>
size_t convert(char buf[], T value)
{
    T i = value;
    char *p = buf;

    do
    {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);

    if (value < 0)
        *p++ = '-';
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

namespace lept_server
{
    class LogStream : noncopyable
    {
    public:
        using Buffer = FixedSizeBuffer<SmallBuffer>;

        inline LogStream &operator<<(bool b)
        {
            buffer_.append(b ? "1" : "0", 1);
            return *this;
        }

        inline LogStream &operator<<(int i)
        {
            format_integer(i);
            return *this;
        }

        inline LogStream &operator<<(unsigned int i)
        {
            format_integer(i);
            return *this;
        }

        inline LogStream &operator<<(short i)
        {
            *this << static_cast<int>(i);
            return *this;
        }

        inline LogStream &operator<<(unsigned short i)
        {
            *this << static_cast<unsigned int>(i);
            return *this;
        }

        inline LogStream &operator<<(long i)
        {
            format_integer(i);
            return *this;
        }

        inline LogStream &operator<<(unsigned long i)
        {
            format_integer(i);
            return *this;
        }

        inline LogStream &operator<<(long long i)
        {
            format_integer(i);
            return *this;
        }

        inline LogStream &operator<<(unsigned long long i)
        {
            format_integer(i);
            return *this;
        }

        LogStream &operator<<(double d);

        LogStream &operator<<(long double d);

        LogStream &operator<<(char c)
        {
            buffer_.append(&c, 1);
            return *this;
        }

        LogStream &operator<<(const char *str)
        {
            if (str)
                buffer_.append(str, strlen(str));
            else
                buffer_.append("(null)", 6);
            return *this;
        }

        LogStream &operator<<(const unsigned char *str)
        {
            return operator<<(reinterpret_cast<const char *>(str));
        }

        LogStream &operator<<(const std::string &str)
        {
            buffer_.append(str.c_str(), str.size());
            return *this;
        }

        const Buffer &buffer() const
        {
            return buffer_;
        }

    private:
        template<typename T>
        void format_integer(T t);

        Buffer buffer_;
        static const int max_numeric_size_ = 32;  // 当Buffer中的可用空间小于这个值时就不能往缓冲区中添加了
    };

    template<typename T>
    void LogStream::format_integer(T t)
    {
        if (buffer_.avail_size() >= max_numeric_size_)
        {
            size_t len = convert(buffer_.current(), t);
            buffer_.add(len);
        }
    }
}


#endif //LEPTSERVER_LOGSTREAM_H
