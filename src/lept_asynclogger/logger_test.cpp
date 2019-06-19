//
// Created by tang on 19-6-19.
// TODO 日志系统压力测试
//
#include "Logger.h"
#include "LogStream.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include <ctime>

using namespace std;
using namespace lept_server;

void stressing_single_thread()
{
    // 100000 lines
    cout << "----------stressing test single thread-----------" << endl;
    for (int i = 0; i < 100000; ++i)
    {
        LOG_INFO << i;
    }
}

int main()
{
    Logger::set_loglevel(Logger::LogLevel::INFO);
    Logger::set_log_file_name("log.log");

    clock_t start, finish;
    start = clock();
    stressing_single_thread();
    finish = clock();

    double duration = (double)(finish - start) / CLOCKS_PER_SEC;

    cout << "Duration = " << duration << endl;

//    sleep(3);

    return 0;
}
