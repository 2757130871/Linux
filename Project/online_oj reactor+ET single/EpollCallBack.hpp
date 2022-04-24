#pragma once

#include <string>
#include <cstring>
#include <functional>

#include <sys/epoll.h>
#include "socket_wrap.hpp"
#include "Util.hpp"

#include "Task.hpp"

#define MAX_CONNECTION 1024
//活跃时间，超过此时间无数据传递就关闭连接
#define MAX_TIMEOUT 6

enum
{
    GET_LINK = 1,   //获取连接
    RECV_DATA = 2,  //读取数据
    SEND_DATA = 3,  //发送数据
};



struct Epoll_CallBack
{
    int fd = -1;   //所监听的fd
    int epfd = -1; //ep模型的根节点
    int event = 0;

    int flag = 0;
    EndPoint *ep = nullptr;

    Epoll_CallBack *start = nullptr;
    //当前的时间戳
    int timestamp = -1;
    //Epoll_CallBack *self = nullptr;

    Epoll_CallBack(int _epfd = -1, int _sock = -1, int _event = 0, int _flag = 0, Epoll_CallBack *_start = nullptr, int stamp = TimeUtil::TimeStamp())
        : epfd(_epfd), fd(_sock), event(_event), flag(_flag), start(_start), timestamp(stamp) {}

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

        int ret = SetNonBlock(new_fd);
        if (!ret)
        {
            LOG(INFO) << "SetN1onBlock Error" << endl;
            return;
        }

        struct epoll_event evt;
        //使用ET边缘触发模式.设置oneshot 防止处理过程中又响应事件导致两个线程同时对一个fd读写
        evt.events = (EPOLLIN | EPOLLET | EPOLLONESHOT);

        int i = 1;
        for (; i < MAX_CONNECTION; i++)
        {
            if (start[i].fd == -1)
                break;
        }

        if (i >= MAX_CONNECTION)
        {
            LOG(FATAL) << "Connection full!!" << std::endl;
            return;
        }

        start[i] = Epoll_CallBack(epfd, new_fd, evt.events, RECV_DATA, start, TimeUtil::TimeStamp());
        evt.data.ptr = &start[i];

        epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &evt);

        LOG(INFO) << "Add connection: " << new_fd << std::endl;
    }

    //删除连接
    static void DelConection(Epoll_CallBack *cb)
    {
        epoll_ctl(cb->epfd, EPOLL_CTL_DEL, cb->fd, NULL);
        close(cb->fd);

        LOG(INFO) << "Delete connection: " << cb->fd << std::endl;
        //释放空间 下次AddConnection使用
        cb->fd = -1;
    }

    void RecvData()
    {
        ep = new EndPoint(fd);

        //更新时间戳
        this->timestamp = TimeUtil::TimeStamp();
        ep->RcvHttpRequest();
        if (!ep->Stop())
        {
            ep->BuildHttpResponse();
            //切换所监听的事件
            Modify_Event();
        }
        else
        {
            delete ep;
            LOG(INFO) << "DEBUG :RECV HttpRequest Error" << std::endl;
            //下树
            DelConection(this);
        }
    }

    void SendData()
    {
        if (ep->SendHttpResponse())
        {
            //发送成功 切换到监听读事件
            Modify_Event();
        }
        else
        {
            LOG(INFO) << "DEBUG :Send HttpResponse Error" << std::endl;
            DelConection(this);
        }

        delete ep;
    }

    //切换监听的事件
    void Modify_Event()
    {
        struct epoll_event evt;

        if (event & EPOLLIN)
        {
            evt.events = EPOLLOUT;
            this->event = EPOLLOUT;
            this->flag = SEND_DATA;
        }
        else if (event & EPOLLOUT)
        {
            evt.events = (EPOLLIN | EPOLLET | EPOLLONESHOT);
            this->event = (EPOLLIN | EPOLLET | EPOLLONESHOT);
            this->flag = RECV_DATA;
        }

        evt.data.ptr = this;
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &evt);
    }
};
