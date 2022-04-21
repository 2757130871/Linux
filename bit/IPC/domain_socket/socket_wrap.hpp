
#include <iostream>


#include <sys/poll.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/un.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include "Util.hpp"

using namespace std;

int Socket_Domain()
{
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0)
    {
        LOG(ERROR) << "SOCKET ERROR" << endl;
        exit(-1);
    }
    return sock;
}

void Bind_Domain(int lsock, const char* path)
{
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    
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

int Accept_Domain(int lsock)
{
    struct sockaddr_un addr;
    int ret = accept(lsock, NULL, NULL);
    if (ret < 0)
    {
        LOG(ERROR) << "ACCEPT ERROR" << endl;
        exit(-1);
    }
    return ret;
}
