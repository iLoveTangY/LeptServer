//
// Created by tang on 19-6-18.
//

#ifndef LEPTSERVER_LOGGING_H
#define LEPTSERVER_LOGGING_H

#include "LogStream.h"
#include <string>

namespace lept_base
{
    class Logger
    {
    public:
        enum LogLevel
        {
            DEBUG,
            INFO,
            WARN,
            ERROR,
            NUM_LOG_LEVELS
        };

        Logger(const char *filename, int line, LogLevel level);

        ~Logger();

        inline LogStream &stream()
        {
            return stream_;
        }

        inline static void set_log_file_name(const std::string &name)
        {
            log_file_name_ = name;
        }

        inline static std::string get_log_file_name()
        {
            return log_file_name_;
        }

        static LogLevel loglevel();

        static void set_loglevel(LogLevel level);

    private:
        void log_time();

        LogStream stream_;
        int line_;  // 产生日志的行号
        std::string file_name_;  // 产生日志的文件名

        static std::string log_file_name_;  // 日志文件名
        LogLevel level_;
    };

#define LOG_DEBUG if (lept_base::Logger::loglevel() <= lept_base::Logger::LogLevel::DEBUG) \
lept_base::Logger(__FILE__, __LINE__, lept_base::Logger::LogLevel::DEBUG).stream()

#define LOG_INFO if (lept_base::Logger::loglevel() <= lept_base::Logger::LogLevel::INFO) \
lept_base::Logger(__FILE__, __LINE__, lept_base::Logger::LogLevel::INFO).stream()

#define LOG_WARN if (lept_base::Logger::loglevel() <= lept_base::Logger::LogLevel::WARN) \
lept_base::Logger(__FILE__, __LINE__, lept_base::Logger::LogLevel::WARN).stream()

#define LOG_ERROR if (lept_base::Logger::loglevel() <= lept_base::Logger::LogLevel::ERROR) \
lept_base::Logger(__FILE__, __LINE__, lept_base::Logger::LogLevel::ERROR).stream()

    extern Logger::LogLevel g_loglevel;  // 当前记录日志的最低等级

    inline Logger::LogLevel Logger::loglevel()
    {
        return g_loglevel;
    }

    inline void Logger::set_loglevel(LogLevel level)
    {
        g_loglevel = level;
    }
}

#endif //LEPTSERVER_LOGGING_H
