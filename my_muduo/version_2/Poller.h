#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "noncopyable.h"
#include "TimeStamp.h"

#include "Channel.h"

class EventLoop;

// Reactor模型中 IO事件分发器的核心模块
class Poller : noncopyable
{
    using ChannelMap = std::unordered_map<int, Channel *>;

public:
    using ChannelList = std::vector<Channel *>;
    Poller(EventLoop *loop);
    virtual ~Poller();

    //调用统一的接口，底层可以为epoll poll select的不同实现
    virtual TimeStamp Poll(int timeoutMs, ChannelList *activeChannels) = 0;

    virtual void UpdateChannel(Channel *channel) = 0;
    virtual void RemoveChannel(Channel *channel) = 0;

    //判断channel是否在ChannelMap中
    bool HasChannel(Channel *channel) const;

    static Poller *NewDefaultPoller(EventLoop *loop);

protected:
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_;
};