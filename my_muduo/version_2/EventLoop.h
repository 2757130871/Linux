#pragma once

#include <functional>
#include <vector>

#include <atomic>
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "TimeStamp.h"

#include "CurrentThread.h"

class Channel;
class Poller;

//事件循环类 可以理解为对epoll的抽象
// 1 Channel
// 2 Poller
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    //开启事件循环
    void Loop();
    void Quit();

    TimeStamp PollReturnTime() const { return pollReturnTime_; }

    //再当前loop中执行
    void RunInLoop(Functor cb);
    //把cb放入queue中，唤醒loop所在线程执行cb
    void QueueInLoop(Functor cb);

    // 每个EventLoop都拥有一个 wakeupFd
    //通过向sub Reactor的wakeupFd写数据实现通知机制  MainReacotr wakeup SubReactor
    void Wakeup();

    // Channel通过EventLoop操作Poller
    void UpdateChannel(Channel *channel);
    void RemoveChannel(Channel *channel);
    bool HasChannel(Channel *channel);

    //判断当前执行线程是否为创建EventLoop的线程
    bool IsInLoopThread() const { return threadId_ == CurrentThread::tid(); };

private:
    // wake up
    void HandleRead();
    //执行回调
    void DoPendingFunctors();

private:
    using ChannelList = std::vector<Channel *>;

    std::atomic_bool looping_; //原子操作 CAS实现
    std::atomic<bool> quit_;   //标识是否退出looping

    const pid_t threadId_; //纪录当前线程的pid

    TimeStamp pollReturnTime_; // poller返回的时间点
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;                           // mainReactor通过Round Robin通知一个SubReactor接收，通过该成员唤醒SubReactor
    std::unique_ptr<Channel> wakeupChannel_; //封装了wakeupFd

    ChannelList activeChannels_; //
    // ChannelList *currentActiveChannel_;

    std::atomic_bool callingPendingFuctors_; //标识当前loop是否有需要执行的回调
    std::vector<Functor> pendingFunctors_;   //存储loop需要执行的所有回调操作
    std::mutex mutex_;                       //保护pendingFunctors_线程安全操作
};