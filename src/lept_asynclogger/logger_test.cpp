//
// Created by tang on 19-6-19.
//
#include "Logger.h"
#include "LogStream.h"
#include "AsyncLogger.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include <ctime>

using namespace std;
using namespace lept_base;

extern AsyncLogger *AsyncLogger_;

#define MESSAGENUM 1000000

void stressing_test()
{
    cout << "----------stressing test-----------" << endl;
    cout << "int test" << endl;
    for (int i = 0; i < MESSAGENUM; ++i)
    {
        LOG_INFO << i;
    }
    cout << "long test" << endl;
    for (long i = 0; i < MESSAGENUM; ++i)
    {
        LOG_INFO << i;
    }
    cout << "double test" << endl;
    for (double i = 0; i < MESSAGENUM; i += 1)
    {
        LOG_INFO << i;
    }
}

void threadFunc()
{
    for (int i = 0; i < MESSAGENUM; ++i)
    {
        LOG_INFO << i;
    }
}

void stressing_multi_threads(int threadNum = 4)
{
    // threadNum * MESSAGENUM lines
    cout << "----------stressing test multi thread-----------" << endl;
    vector<shared_ptr<Thread>> vsp;
    for (int i = 0; i < threadNum; ++i)
    {
        shared_ptr<Thread> tmp(new Thread(threadFunc, "testFunc"));
        vsp.push_back(tmp);
    }
    for (int i = 0; i < threadNum; ++i)
    {
        vsp[i]->start();
    }

    for (int i = 0; i < threadNum; ++i)
    {
        vsp[i]->join();  // 这种测试方式有问题？
    }
//    sleep(3);
}

int main()
{
    Logger::set_loglevel(Logger::LogLevel::INFO);
    Logger::set_log_file_name("/dev/null");

    clock_t start, finish;
    start = clock();
    stressing_multi_threads();
    finish = clock();

    delete AsyncLogger_;  // 特殊用途

    double duration = (double)(finish - start) / CLOCKS_PER_SEC;

    cout << MESSAGENUM * 4 / duration << " msg/s" << endl;

    return 0;
}
