#include <signal.h>

#include "CallBack.hpp"

#define BACKLOG 4


//使用EPOLLET模型 和 ONESHOT
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Usage: ./server port" << endl;
        exit(-1);
    }

    //防止服务器被SIGPIPE信号终止
    signal(SIGPIPE, SIG_IGN);

    //经典三联
    int port = atoi(argv[1]);
    int lsock = Socket();
    Bind(lsock, port);
    Listen(lsock, BACKLOG);

    //创建eoll模型
    int epfd = epoll_create(1);

    //添加lsock
    struct epoll_event ent;
    ent.events = EPOLLIN;

    ent.data.ptr = new CallBack(epfd, lsock, ent.events, GET_LINK);
    epoll_ctl(epfd, EPOLL_CTL_ADD, lsock, &ent);

    struct epoll_event evs[1024];

    while (1)
    {
        int nready = epoll_wait(epfd, evs, 1024, -1);
        if (nready < 0)
        {
            LOG(FATAL) << "epoll_wait Error !" << endl;
            continue;
        }
        
        for (int i = 0; i < nready; i++)
        {
            CallBack *call_back = static_cast<CallBack *>(evs[i].data.ptr);
            (*call_back)(); //调用回调
        }
    }
}
