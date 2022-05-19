#include "EpollPoller.h"

#include <cstring>

EpollPoller::EpollPoller(EventLoop *loop)
    : Poller(loop), epollFd_(epoll_create1(EPOLL_CLOEXEC)), events_(kInitEventListSize)
{
    if (epollFd_ < 0)
    {
        LOG(FATAL) << "epoll create error!" << std::endl;
        exit(0);
    }
}

EpollPoller::~EpollPoller()
{
    close(epollFd_);
}

//调用统一的接口，底层可以为epoll poll select的不同实现
//返回发生的具体时间点
TimeStamp EpollPoller::Poll(int timeoutMs, ChannelList *activeChannels)
{
    int numEvents = ::epoll_wait(epollFd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;

    TimeStamp now(TimeStamp::Now());
    if (numEvents > 0)
    {
        FillActiveChannels(numEvents, activeChannels);
        //扩容
        if (numEvents == events_.size())
            events_.resize(events_.size() * 2);
    }
    else if (numEvents == 0)
    {
        LOG(INFO) << "epoll_wait timeout !!" << std::endl;
    }
    else
    {
        //可能执行系统调用收到信号被外部中断 EINTR
        if (errno != EINTR)
        {
            errno = saveErrno;
            LOG(ERROR) << strerror(errno) << std::endl;
        }
    }

    return now;
}

//填写活跃的连接EventLoop获取到所有epoll_wait返回的活跃连接
void EpollPoller::FillActiveChannels(int numsEvents, ChannelList *activeChannels) const
{
    for (int i = 0; i < numsEvents; i++)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);

        channel->SetREvents(events_[i].events);
        activeChannels->push_back(channel);
    }
}

// Channel update/delete => EventLoop => Poller UpdataChannel/RemoveChannel
void EpollPoller::UpdateChannel(Channel *channel)
{
    int fd = channel->Get_Fd();
    int index = channel->GetIndex();
    std::cout << "DEBUG: index: " << index << std::endl;

    if (index == kNew || index == kDeleted)
    {
        if (index == kNew)
        {
            channels_[fd] = channel;
        }
        channel->SetIndex(kAdded);
        Update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        //更新已存在的channel （DEL/MOD）
        if (channel->IsNoneEvent())
        {
            //对所有事件已不感兴趣了
            Update(EPOLL_CTL_DEL, channel);
            channel->SetIndex(kDeleted);
        }
        else
        {
            Update(EPOLL_CTL_MOD, channel);
        }
    }
}

//把Channel从Poller中移除
void EpollPoller::RemoveChannel(Channel *channel)
{
    int fd = channel->Get_Fd();
    channels_.erase(fd);

    //从Epoll模型中删除
    if (channel->GetIndex() == kAdded)
        Update(EPOLL_CTL_DEL, channel);

    //设置回未添加状态
    channel->SetIndex(kNew);
}

//更新Channel感兴趣的事件 对epoll add/mod/del 的具体操作
void EpollPoller::Update(int operation, Channel *channel)
{
    int fd = channel->Get_Fd();
    struct epoll_event evt;
    memset(&evt, 0, sizeof(evt));
    evt.events = channel->GetEvents();

    evt.data.ptr = channel;

    if (::epoll_ctl(epollFd_, operation, fd, &evt) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG(ERROR) << " EpollPoller::Update  Error: EPOLL_CTL_DEL" << std::endl;
        }
        else
        { //无法往epoll模型上添加/修改fd属于fatal错误
            LOG(FATAL) << " EpollPoller::Update Error: EPOLL_CTL_ADD/MOD" << std::endl;
            exit(0);
        }
    }
}