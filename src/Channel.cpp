//
// Created by tang on 19-6-28.
//

#include "Channel.h"

lept_server::Channel::Channel(EventLoop *loop)
        : loop_(loop), events_(0), last_events_(0)
{
}

lept_server::Channel::Channel(EventLoop *loop, int fd)
        : loop_(loop), fd_(fd), events_(0), last_events_(0)
{
}

lept_server::Channel::~Channel()
{
}
