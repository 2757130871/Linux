#include "Poller.h"
#include "EpollPoller.h"

Poller *Poller::NewDefaultPoller(EventLoop *loop)
{
    return new EpollPoller(loop);
}