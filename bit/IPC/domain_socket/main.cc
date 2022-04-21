#define _GNU_SOURCE 1

#include <signal.h>
#include <cstring>
#include "socket_wrap.hpp"

#define BACKLOG 4
#define MAX_LINK 128

int main(int argc, char *argv[])
{
 
    signal(SIGPIPE, SIG_IGN); //防止服务器被SIGPIPE信号终止

    int lsock = Socket_Domain();

    Bind_Domain(lsock, "./sock.c");
    //
    Listen(lsock, 128); //?????
    int cfd = Accept_Domain(lsock);
    char buf[255];
    while (1)
    {
        int rd = recv(cfd, buf, sizeof(buf) - 1, 0);
        if (rd < 0)
        {
            cout << "error " << endl;
            exit(-1);
        }
        else if (rd == 0)
        {
            cout << "client close" << endl;
            close(cfd);
            exit(-1);
        }
        else if (rd > 0)
        {
            buf[rd] = 0;
            cout << "recv: " << buf << endl;
        }
    }
}
