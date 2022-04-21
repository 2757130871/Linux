
#include "socket_wrap.hpp"
#include <signal.h>

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        std::cout << "usage:" << endl;
        exit(-1);
    }

    signal(SIGPIPE, SIG_IGN);

    int port = atoi(argv[1]);

    int lsock = Socket();
    Bind(lsock, port);
    Listen(lsock, 5);

    int sock = Accept(lsock);

    char c = 'a';
    char buf[255];
    while (1)
    {
        if (c == 'z')
            c = 'a';
        int sd = send(sock, &c, 1, 0);
        if (sd > 0)
        {
            cout << "sned 1 bit" << endl;
        }
        else if (sd < 0)
        {
            if(errno == EPIPE)
                cout << "ERROR: " << strerror(errno) << endl;
        }
        else if (sd == 0)
        {
            cout << "?????" << endl;
        }
        c++;

        sleep(1);
    }
}
