#include <memory>

//
// Created by tang on 19-6-17.
//

#include "LogFile.h"

lept_server::LogFile::LogFile(const std::string &filename, int flush_every_N)
        : flush_every_N_(flush_every_N), count_(0)
{
    p_mutex_ = std::make_unique<MutexLock>();
    p_file_ = std::make_unique<AppendableFile>(filename);
}

int lept_server::LogFile::flush()
{
    MutexLockGuard lock(*p_mutex_);
    return p_file_->flush();
}

size_t lept_server::LogFile::append(const char *logline, size_t len)
{
    MutexLockGuard lock(*p_mutex_);
    auto n = append_unlocked(logline, len);
    return n;
}

size_t lept_server::LogFile::append_unlocked(const char *logline, size_t len)
{
    auto n = p_file_->append(logline, len);
    ++count_;
    if (count_ >= flush_every_N_)
    {
        count_ = 0;
        p_file_->flush();
    }
    return n;
}
