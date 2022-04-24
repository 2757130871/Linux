#pragma once
#include <pthread.h>

#include "socket_wrap.hpp"

#define BACKLOG 4

class TcpServer
{
public:
    static TcpServer *GetInstance(int _port)
    {
        if (nullptr == tcp_server)
        {
            pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_lock(&lock);

            if (nullptr == tcp_server)
            {
                tcp_server = new TcpServer(_port);
                tcp_server->init();
            }

            pthread_mutex_unlock(&lock);
        }

        return tcp_server;
    }

    int GetLsock()
    {
        return lsock;
    }

private:
    TcpServer(int _port) : lsock(-1), port(_port){};
    TcpServer(const TcpServer &server){};

    void init()
    {
        lsock = Socket();
        Bind(lsock, port);
        Listen(lsock, BACKLOG);
    }

private:
    static TcpServer *tcp_server;
    int lsock;
    int port;
};

TcpServer *TcpServer::tcp_server = nullptr;
