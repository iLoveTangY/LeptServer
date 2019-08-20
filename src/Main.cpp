//
// Created by tang on 19-7-3.
//

#include "EventLoop.h"
#include "Server.h"
#include "Logger.h"
#include <getopt.h>
#include <string>

using namespace std;
using namespace lept_base;
using namespace lept_server;

int main(int argc, char *argv[])
{
    int threadNum = 4;
    int port = 8080;
    std::string logPath = "./Lept_Server.log";

    // parse args
    int opt;
    const char *str = "t:l:p:";
    while ((opt = getopt(argc, argv, str))!= -1)
    {
        switch (opt)
        {
            case 't':
            {
                threadNum = atoi(optarg);
                break;
            }
            case 'l':
            {
                logPath = optarg;
                if (logPath.size() < 2)
                {
                    printf("invalid log path\n");
                    abort();
                }
                break;
            }
            case 'p':
            {
                port = atoi(optarg);
                break;
            }
            default: break;
        }
    }

    Logger::set_log_file_name(logPath);
    // STL库在多线程上应用
#ifndef _PTHREADS
    LOG_DEBUG << "_PTHREADS is not defined !";
#endif
    EventLoop mainLoop;  // 主事件循环
    Server myHTTPServer(&mainLoop, threadNum, port);
    myHTTPServer.start();

    return 0;
}