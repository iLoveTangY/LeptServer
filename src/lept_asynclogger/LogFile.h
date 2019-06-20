//
// Created by tang on 19-6-17.
//

#ifndef LEPTSERVER_LOGFILE_H
#define LEPTSERVER_LOGFILE_H

#include <memory>
#include <string>

#include "AppendableFile.h"
#include "noncopyable.h"
#include "MutexLock.h"

namespace lept_base
{
    class LogFile : noncopyable
    {
    public:
        explicit LogFile(const std::string& filename, int flush_every_N = 1024);

        size_t append(const char* logline, size_t len);

        int flush();

    private:
        size_t append_unlocked(const char* logline, size_t len);
        int flush_every_N_;  // 每执行Append flush_every_n次执行一次文件的flush操作
        int count_;  // 目前执行的Append函数次数
        std::unique_ptr<MutexLock> p_mutex_;
        std::unique_ptr<AppendableFile> p_file_;
    };
}


#endif //LEPTSERVER_LOGFILE_H
