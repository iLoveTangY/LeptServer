//
// Created by tang on 19-6-17.
//

#ifndef LEPTSERVER_APPENDABLEFILE_H
#define LEPTSERVER_APPENDABLEFILE_H

#include <string>

#include <cstdio>

#include "noncopyable.h"

namespace lept_server
{
    class AppendableFile : noncopyable
    {
    public:
        explicit AppendableFile(std::string filename);
        ~AppendableFile();

        // 往文件中追加logline，长度为len，返回成功写入的字符数
        size_t append(const char *logline, size_t len);

        int flush();

    private:
        inline size_t write(const char *logline, size_t len)
        {
            // fwrite的无锁版本，可以保证只有一个线程会调用本函数，因此这里使用无锁版本加快速度
            return fwrite_unlocked(logline, 1, len, fp_);
        }

        FILE *fp_;
        char buffer_[64*1024];  // 64kb的缓冲区
    };
}


#endif //LEPTSERVER_APPENDABLEFILE_H
