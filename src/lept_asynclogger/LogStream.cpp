//
// Created by tang on 19-6-17.
//

#include <algorithm>
#include <iostream>
#include "LogStream.h"

const char digits[] = "9876543210123456789";  // 为了负数时也通用定义成这个样子
const char *zero = digits + 9;

lept_base::LogStream &lept_base::LogStream::operator<<(double d)
{
    if (buffer_.avail_size() >= max_numeric_size_)
    {
        int len = snprintf(buffer_.current(), max_numeric_size_, "%.12g", d);
        buffer_.add(static_cast<size_t>(len));
    }
    return *this;
}

lept_base::LogStream &lept_base::LogStream::operator<<(long double d)
{
    if (buffer_.avail_size() >= max_numeric_size_)
    {
        int len = snprintf(buffer_.current(), max_numeric_size_, "%.12Lg", d);
        buffer_.add(static_cast<size_t>(len));
    }
    return *this;
}
