#pragma once

#include <vector>

#include <unistd.h>

#include "Poller.h"
#include "base/Util.hpp"

const int kNew = -1;    //未添加入epoll模型
const int kAdded = 1;   //已添加入epoll模型
const int kDeleted = 2; //已从epoll模型中删除

/*
 *   epoll_create
 *   epoll_ctl  (ADD/MOD/DEL)
 *   epoll_wait
 */
class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop *loop);

    virtual ~EpollPoller() override;
    //调用统一的接口，底层可以为epoll poll select的不同实现
    virtual TimeStamp Poll(int timeoutMs, ChannelList *activeChannels) override;
    virtual void UpdateChannel(Channel *channel) override;
    virtual void RemoveChannel(Channel *channel) override;

private:
    //填写活跃的连接
    void FillActiveChannels(int numsEvents, ChannelList *activeChannels) const;

    //更新Channel感兴趣的事件
    void Update(int operation, Channel *channel);

private:
    static const int kInitEventListSize = 16; // events_初始化长度
    using EventList = std::vector<epoll_event>;

    int epollFd_;
    EventList events_;
};
