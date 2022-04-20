#include <signal.h>
#include <cstring>
#include "socket_wrap.hpp"

#define BACKLOG 4
#define MAX_LINK 128

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Usage: ./server port" << endl;
        exit(-1);
    }

    signal(SIGPIPE, SIG_IGN); //防止服务器被SIGPIPE信号终止

    int port = atoi(argv[1]);

    int lsock = Socket();
    Bind(lsock, port);
    Listen(lsock, BACKLOG);

    struct pollfd fds[MAX_LINK];
    memset(fds, 0, sizeof(fds));

    for (int i = 0; i < MAX_LINK; i++)
        fds[i].fd = -1;

    fds[0].fd = lsock;
    fds[0].events |= POLL_IN;
    int index = 1;

    char buf[255];

    while (1)
    {
        poll(fds, MAX_LINK, -1); //阻塞式等待
        cout << "DEBUG" << endl;
        for (int i = 0; i < MAX_LINK; i++)
        {
            if (fds[i].fd != -1 && fds[i].revents & POLL_IN)
            {
                if (fds[i].fd == lsock)
                {
                    if (index >= MAX_LINK)
                    {
                        cout << "connection full" << endl;
                        continue;
                    }
                    int new_fd = Accept(lsock);
                    fds[index].fd = new_fd;
                    fds[index++].events |= POLL_IN;

                    cout << "add connection: " << new_fd << endl;
                    continue;
                }

                //处理逻辑
                int rd = read(fds[i].fd, buf, sizeof(buf) - 1);
                if (rd == 0) // 断开连接
                {
                    fds[i].fd = -1;
                    close(i);
                    cout << "client close" << endl;
                }
                else if (rd < 0)
                { //错误
                    cout << "recv error" << endl;
                }
                else if (rd > 0)
                {
                    buf[rd] = 0;
                    cout << "client: " << buf << endl;
                }
            }
        }
    }
}
