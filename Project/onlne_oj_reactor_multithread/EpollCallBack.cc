#pragma once

#include <string>
#include <cstring>
#include <functional>

#include <sys/epoll.h>
#include "socket_wrap.hpp"
#include "Util.hpp"

#include "Protocol.hpp"

#define MAX_CONNECTION 1024
//活跃时间，超过此时间无数据传递就关闭连接
#define MAX_TIMEOUT 60

//子进程数量
#define PROC_NUM 4

enum
{
    GET_LINK = 1,
    RECV_DATA = 2,
    SEND_DATA = 3,
};

struct Epoll_CallBack
{
    int fd = -1;   //所监听的fd
    int epfd = -1; // ep模型的根节点
    int event = 0;

    int flag = 0;
    EndPoint *ep = nullptr;

    Epoll_CallBack *start = nullptr;
    //当前的时间戳
    int timestamp = -1;
    // Epoll_CallBack *self = nullptr;

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

    //添加链接
    void AddConection()
    {
        int new_fd = Accept(fd);
        // int sum = 4;
        // char buf[4];
        // char *cur = buf;
        // while (sum)
        // {
        //     int rd = read(rd, cur++, 1);
        //     sum -= rd;
        // }

        // int new_fd = *(int *)buf;
        // DEBUG
        LOG(INFO) << "pid " << getpid() << " new fd " << new_fd << endl;

        //设置新连接非阻塞
        int ret = SetNonBlock(new_fd);
        if (!ret)
        {
            LOG(INFO) << "SetN1onBlock Error" << endl;
            return;
        }

        struct epoll_event evt;
        //使用ET边缘触发模式.设置oneshot 一次处理流程中不会被其他线程处理
        evt.events = (EPOLLIN | EPOLLET);

        //寻找新位置存放新连接
        int i = 1;
        for (; i < MAX_CONNECTION; i++)
        {
            if (start[i].fd == -1)
                break;
        }
        //判断连接数是否到达上限
        if (i >= MAX_CONNECTION)
        {
            LOG(FATAL) << "Connection full!!" << std::endl;
            return;
        }

        start[i] = Epoll_CallBack(epfd, new_fd, evt.events, RECV_DATA, start, TimeUtil::TimeStamp());
        evt.data.ptr = &start[i];

        epoll_ctl(epfd, EPOLL_CTL_ADD, new_fd, &evt);

        LOG(INFO) << "Add connection: " << new_fd << std::endl;
        ResetEvent();
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

            ep->SendHttpResponse();
            delete ep;
            DelConection(this);
            return;

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
            // if (ep->IsKeepAlive())
            // {
            //     // Modify_Event();
            //     LOG(ERROR) << "长连接测试： true" << endl;
            // }
            // else
            // {
            //     LOG(ERROR) << "长连接测试： false" << endl;
            //     //关闭连接
            // }

            //发送成功，切换监听读事件 保持长连接
        }
        else
        {
            LOG(INFO) << "DEBUG :Send HttpResponse Error" << std::endl;
        }

        delete ep;
        DelConection(this);
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
            evt.events = (EPOLLIN | EPOLLET);
            this->event = (EPOLLIN | EPOLLET);
            this->flag = RECV_DATA;
        }

        evt.data.ptr = this;
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &evt);
    }

    //重置 EPOLLONESHOT
    void ResetEvent()
    {
        struct epoll_event evt;
        evt.data.ptr = this;
        evt.events = (EPOLLIN | EPOLLONESHOT);
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &evt);
    }
};
