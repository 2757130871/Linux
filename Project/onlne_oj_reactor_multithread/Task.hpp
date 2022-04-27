#pragma once
#include <iostream>

#include "Protocol.hpp"
#include "EpollCallBack.hpp"

class Task
{
public:
    Task(Epoll_CallBack *cb) : handler(cb) {}
    ~Task() {}

    void ProcessOn()
    {
        (*handler)();
    }

private:
    int sock;
    Epoll_CallBack *handler; //设置回调
};
