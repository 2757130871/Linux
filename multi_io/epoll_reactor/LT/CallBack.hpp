#pragma once

#include <string>
#include <functional>

#include <sys/epoll.h>
#include "socket_wrap.hpp"
#include "Util.hpp"

enum
{
    GET_LINK = 1,
    RECV_DATA = 2,
    SEND_DATA = 3,
};

struct CallBack
{
    int fd;   //所监听的fd
    int epfd; //ep模型的根节点
    int event;

    int flag;

    std::string recv_str;
    std::string send_str;

    CallBack(int _epfd, int _sock, int _event, int _flag) : epfd(_epfd), fd(_sock), event(_event), flag(_flag) {}

    void operator()()
    {
        switch (flag)
        {
        case GET_LINK:
            AddConection();
            break;
        case RECV_DATA:
            RecvData();
            break;
        case SEND_DATA:
            SendData();
            break;
        }
    }

    void AddConection()
    {
        int new_fd = Accept(fd);

        struct epoll_event evt;
        evt.events = EPOLLIN;

        evt.data.ptr = new CallBack(epfd, new_fd, evt.events, RECV_DATA);
        epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &evt);

        LOG(INFO) << "Add connection: " << new_fd << std::endl;
    }

    //删除链接
    void DelConection()
    {
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
        close(fd);
        LOG(INFO) << "Delete connection: " << fd << std::endl;
        delete this;
    }

    void RecvData()
    {
        char buf[255];
        int rd = read(fd, buf, sizeof(buf) - 1);

        if (rd < 0)
        {
            LOG(INFO) << "recv data from fd error, delete fd: " << fd << endl;
            DelConection();
        }
        else if (rd == 0)
        {
            LOG(INFO) << "client close: " << fd << endl;
            DelConection();
        }
        else if (rd > 0)
        {
            buf[rd] = 0;
            recv_str += buf;
            //接收完数据，切换
            Modify_Event();
        }
    }

    void SendData()
    {
        int sd = send(fd, recv_str.c_str(), recv_str.size(), 0);
        recv_str.clear();
        //如果读取完毕，切换监听写事件。如果发送完毕，监听读事件
        Modify_Event();
    }

    //切换监听的事件
    void Modify_Event()
    {
        struct epoll_event evt;

        if (event == EPOLLIN)
        {
            evt.events = EPOLLOUT;
            this->event = EPOLLOUT;
            this->flag = SEND_DATA;
        }
        else if (event == EPOLLOUT)
        {
            evt.events = EPOLLIN;
            this->event = EPOLLIN;
            this->flag = RECV_DATA;
        }

        evt.data.ptr = this;
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &evt);
    }
};
