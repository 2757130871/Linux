#pragma once
#include <iostream>
#include "Protocol.hpp"

class Task
{

public:
    Task(int _sock) : sock(_sock) {}
    ~Task() {}

    void ProcessOn()
    {
        handler(sock);
    }

private:
    int sock;
    CallBack handler; //设置回调
};
