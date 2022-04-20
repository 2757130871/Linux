#include "socket_wrap.hpp"

#define BACKLOG 4

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Usage: ./server port" << endl;
        exit(-1);
    }
    int port = atoi(argv[1]);

    int lsock = Socket();
    Bind(lsock, port);
    Listen(lsock, BACKLOG);

    int maxi = lsock + 1;
    fd_set r_set;
    FD_ZERO(&r_set);

    FD_SET(lsock, &r_set);

    char buf[255];

    while (1)
    {
        fd_set all_set = r_set; //备份

        select(maxi, &r_set, NULL, NULL, 0); //阻塞式等待

        int next_maxi = maxi;
        for (int i = lsock; i < maxi; i++)
        {
            if (FD_ISSET(i, &r_set))
            {
                if (i == lsock)
                {
                    int new_fd = Accept(lsock);
                    FD_SET(new_fd, &all_set);
                    cout << "add connection: " << new_fd << endl;
                    next_maxi = std::max(new_fd + 1, next_maxi);
                    continue;
                }

                //处理逻辑
                int rd = read(i, buf, sizeof(buf) - 1);
                if (rd == 0) // 断开连接
                {
                    FD_CLR(i, &all_set);
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

        r_set = all_set;
        maxi = next_maxi;
    }
}
