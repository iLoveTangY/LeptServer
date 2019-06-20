//
// Created by tang on 19-6-18.
//

#include "Logger.h"
#include "AsyncLogger.h"

#include <ctime>
#include <pthread.h>
#include <sys/time.h>
#include <iostream>

using std::cout;
using std::endl;

using namespace lept_server;

static pthread_once_t once_control = PTHREAD_ONCE_INIT;
AsyncLogger *AsyncLogger_;  // 为了测试日志库的性能，将其设为了非静态，实际使用时使用静态变量较好


Logger::LogLevel init_loglevel()
{
    if (::getenv("LEPT_LOG_DEBUG"))
        return Logger::LogLevel::DEBUG;
    else
        return Logger::LogLevel::INFO;
}

// 当前记录日志的最低等级
Logger::LogLevel lept_server::g_loglevel = init_loglevel();

// 日志文件名
std::string Logger::log_file_name_ = "log.log";

const char *log_level_name[Logger::NUM_LOG_LEVELS] =
        {
                "DEBUG ",
                "INFO  ",
                "WARN  ",
                "ERROR ",
        };

void once_init()
{
    AsyncLogger_ = new AsyncLogger(Logger::get_log_file_name());
    AsyncLogger_->start();
}

// 此函数在Logger对象被析构时将日志写入到AsyncLogging的缓冲区中
// 再由AsyncLogging异步将其写入到文件
void output(const char *msg, size_t len)
{
    pthread_once(&once_control, once_init);
    AsyncLogger_->append(msg, len);
}

Logger::Logger(const char *filename, int line, LogLevel level) : file_name_(filename), line_(line), level_(level)
{
    log_time();  // 构造对象时先记录下当前时间
    stream_ << " -- " << log_level_name[level_] << file_name_ << ":" << line_ << "\n";  // 写入文件名行号等信息
}

lept_server::Logger::~Logger()
{
    const LogStream::Buffer &buf(stream().buffer());
    output(buf.data(), buf.length());
//    AsyncLogger_->stop();
}

void Logger::log_time()
{
    timeval tv{};
    time_t time;
    char str_t[26] = {0};
    gettimeofday(&tv, nullptr);
    time = tv.tv_sec;
    tm *p_time = localtime(&time);
    strftime(str_t, 26, "\n%Y-%m-%d %H:%M:%S", p_time);
    stream_ << str_t;
}
