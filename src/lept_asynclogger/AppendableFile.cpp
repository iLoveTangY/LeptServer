//
// Created by tang on 19-6-17.
//

#include "AppendableFile.h"

lept_server::AppendableFile::AppendableFile(std::string filename)
        : fp_(fopen(filename.c_str(), "ae"))
{
    setbuffer(fp_, buffer_, sizeof(buffer_));
}

lept_server::AppendableFile::~AppendableFile()
{
    fclose(fp_);
}

int lept_server::AppendableFile::flush()
{
    return fflush(fp_);
}

size_t lept_server::AppendableFile::append(const char *logline, size_t len)
{
    size_t n = write(logline, len);
    size_t remain = len - n;
    while (remain > 0)
    {
        size_t wrote = write(logline + n, remain);
        if (wrote == 0)
        {
            int err = ferror(fp_);
            if (err)
                fprintf(stderr, "AppendableFile::append() failed!\n");
            break;
        }
        n += wrote;
        remain = len - n;
    }
    return n;
}
