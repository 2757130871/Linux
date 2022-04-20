
#include <iostream>

#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

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
    struct sockaddr_in addr;
    int ret = accept(lsock, NULL, NULL);
    if (ret < 0)
    {
        LOG(ERROR) << "ACCEPT ERROR" << endl;
        exit(-1);
    }
    return ret;
}
