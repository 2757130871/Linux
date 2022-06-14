#include <iostream>
#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <error.h>
#include <errno.h>
#include <sys/socket.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

int main()
{
    int fds[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0)
    {
        cout << "socketpair error" << endl;
        exit(1);
    }

    if (fork() > 0) //父进程
    {

        while (1)
        {
            write(fds[1], "abcde", 5);
            sleep(1);
        }
    }
    else
    {

        char buf[255];
        while (1)
        {
            int rd = read(fds[0], &buf, sizeof(buf) - 1);
            if (rd <= 0)
            {
                cout << "server exit" << endl;
                exit(0);
            }
            else
            {
                buf[rd] = 0;
                cout << buf << endl;
            }
        } // end of whle
    }
}

void test1()
{
    if (fork() > 0) //父进程
    {
        int server_push = open("server_push", O_WRONLY);
        while (1)
        {

            write(server_push, "hello", 5);
            sleep(1);
        }
    }
    else
    {
        int client_pop = open("server_push", O_RDONLY);

        char buf[255];
        while (1)
        {
            int rd = read(client_pop, &buf, sizeof(buf) - 1);
            if (rd <= 0)
            {
                cout << "server exit" << endl;
                exit(0);
            }
            else
            {
                buf[rd] = 0;
                cout << buf << endl;
            }
        } // end of whle
    }
}
