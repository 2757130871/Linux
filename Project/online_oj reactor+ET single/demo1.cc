
#include <signal.h>
#include <sys/epoll.h>

#include "socket_wrap.hpp"

#define BACKLOG 4

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

    int epfd = epoll_create(1024);
    struct epoll_event ent;
    ent.events |= EPOLLIN;
    ent.data.fd = lsock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lsock, &ent);

    struct epoll_event evnts[1024];

    char buf[255];

    while (1)
    {
        int nready = epoll_wait(epfd, evnts, 1024, -1);
        for (int i = 0; i < nready; i++)
        {
            if (evnts[i].events & EPOLLIN)
            {
                //添加链接
                if (evnts[i].data.fd == lsock)
                {
                    int new_fd = Accept(lsock);
                    struct epoll_event n;
                    n.events |= EPOLLIN;
                    n.events |= EPOLLONESHOT;
                    n.data.fd = new_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &n);
                    continue;
                }

                //数据处理
                int fd = evnts[i].data.fd;
                int rd = read(fd, buf, sizeof(buf) - 1);

                if (rd < 0)
                { //错误
                    cout << "recv error" << endl;
                }
                else if (rd > 0)
                {
                    buf[rd] = 0;
                    cout << "client: " << buf << endl;
                }
                else if (rd == 0)
                {
                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    cout << "client close" << endl;
                }
            }
        }
    }
}
