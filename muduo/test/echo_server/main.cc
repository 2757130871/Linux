
#include "test.hpp"

int main()
{
    muduo::net::EventLoop loop;

    CharServer server(&loop, {"127.0.0.1", 8080}, "CharServer");
    server.Start(); // epoll_ctl 添加listen_fd
    loop.loop();    //  epoll wait 阻塞等待
}
