#pragma once
#include <iostream>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include <arpa/inet.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include "Util.hpp"

using namespace std;

int Socket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        LOG(ERROR) << "SOCKET ERROR" << endl;
        exit(-1);
    }
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    return sock;
}

void Bind(int lsock, int port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(lsock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        LOG(ERROR) << "BIND ERROR" << endl;
        exit(-1);
    }
}

void Listen(int lsock, int backlog)
{
    if (listen(lsock, backlog) != 0)
    {
        LOG(ERROR) << "LISTEN ERROR" << endl;
        exit(-1);
    }
}

int Accept(int lsock)
{
    int ret = accept(lsock, NULL, NULL);
    if (ret < 0)
    {
        LOG(ERROR) << "ACCEPT ERROR" << endl;
        exit(-1);
    }
    return ret;
}

void Connect(int lsock, int port, const char *serv_ip)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(serv_ip);
    socklen_t len = sizeof(addr);
    if (connect(lsock, (struct sockaddr *)&addr, len) != 0)
    {
        LOG(ERROR) << "CONNECT ERROR" << endl;
        exit(-1);
    }
}

bool SetNonBlock(int fd)
{
    int fl = fcntl(fd, F_GETFL);
    if (fl < 0)
    {
        std::cout << "fcntl error" << endl;
        return false;
    }

    int ret = fcntl(fd, F_SETFL, fl | O_NONBLOCK);
    if (ret < 0)
    {
        std::cout << "fcntl error" << endl;
        return false;
    }
    return true;
}