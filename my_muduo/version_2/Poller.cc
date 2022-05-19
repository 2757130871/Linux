
#include "Poller.h"

Poller::Poller(EventLoop *loop) : ownerLoop_(loop) {}

bool Poller::HasChannel(Channel *channel) const
{
    auto it = channels_.find(channel->Get_Fd());
    return it != channels_.end() && it->second == channel;
}

Poller::~Poller() {}