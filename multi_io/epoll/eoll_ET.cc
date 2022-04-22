#include <signal.h>
#include <string>

#include <sys/epoll.h>

#include "socket_wrap.hpp"

#define BACKLOG 4

//读取缓冲区大小
#define BUF_SIZE 4

//使用EPOLLET模型 和 ONESHOT
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

    int epfd = epoll_create(1);
    struct epoll_event ent;
    ent.events |= EPOLLIN;
    ent.data.fd = lsock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, lsock, &ent);

    struct epoll_event evs[1024];
    char buf[BUF_SIZE];
    while (1)
    {
        int nready = epoll_wait(epfd, evs, 1024, -1);
        for (int i = 0; i < nready; i++)
        {
            int cfd = evs[i].data.fd;
            if (cfd == lsock)
            {
                //添加链接

                int new_fd = Accept(lsock);
                SetNonBlock(new_fd); //设置非阻塞
                cout << "new link: " << new_fd << endl;

                struct epoll_event e;
                e.events = (EPOLLIN | EPOLLET); // EPOLLONESHOT
                e.data.fd = new_fd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &e);
                continue;
            }

            string str;
            int rd;

            while (true)
            {
                rd = read(cfd, buf, BUF_SIZE - 1);
                cout << "DEBUG: " << rd << endl;
                if (rd > 0 || (rd == -1 && errno == EAGAIN)) //有数据情况
                {
                    if (rd != -1)
                    {
                        buf[rd] = 0;
                        str += buf;
                    }

                    if (rd < BUF_SIZE - 1)
                    {
                        // ent.events = (EPOLLIN | EPOLLONESHOT | EPOLLET);
                        // ent.data.fd = cfd;
                        // epoll_ctl(epfd, EPOLL_CTL_MOD, cfd, &ent);
                        cout << "client send: " << str << endl;
                        cout << "size: " << str.size() << endl;
                        break;
                    }
                }
                else if (rd == 0)
                {
                    //对端断开链接
                    std::cout << "client close" << endl;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
                    close(cfd);
                    break;
                }
                else if (rd < 0)
                {
                    std::cout << "recv error" << endl;
                    break;
                }
            }
        }
    }
}
