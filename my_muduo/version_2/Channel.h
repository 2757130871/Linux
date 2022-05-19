#pragma once

#include <sys/epoll.h>

#include <functional>
#include <memory>

#include "noncopyable.h"
#include "TimeStamp.h"

#include "EventLoop.h"

// Channel 通道
//封装了fd 和event（EPOLLIN/OUT）
//绑定了poller返回的事件处理
class Channel : noncopyable
{
public:
    using EventCallBack = std::function<void()>;
    using ReadEventCallBack = std::function<void(TimeStamp)>;

    Channel(EventLoop *loop, int fd);

    //析构操作必须是EventLoop所再的线程负责析构此Channel
    ~Channel();

    // fd通过poller通知的事件，调用相对应的回调
    void HandleEvent(TimeStamp recviveTime);

    //设置回调对象
    void SetReadCallBack(ReadEventCallBack cb);
    void SetWriteCallBack(EventCallBack cb);
    void SetCloseCallBack(EventCallBack cb);
    void SetErrorCallBack(EventCallBack cb);

    //防止cannel已经被手动remove还在执行回调
    void Tie(const std::shared_ptr<void> &obj);

    int Get_Fd() const;
    int GetEvents() const;
    void SetREvents(int revents);
    bool IsNonEvent() const;

    void EnableReading();
    void DisableReading();
    void EnableWriting();
    void DisableWrirting();
    void DisableAll();

    bool IsNoneEvent();
    bool IsReading();
    bool IsWriting();

    int GetIndex();
    void SetIndex(int idx);

    EventLoop *OwnerLoop();

    //在Channel所属的EventLoop中,删除当前channel
    void Remove();

private:
    //通过EventLoop通知Poller更新感兴趣的事件 epoll_ctl
    void Update();

    void HandleEventWithGuard(TimeStamp recviveTime);

private:
    static const int kNoneEvent;
    static const int kWriteEvent;
    static const int kReadEvent;

    EventLoop *loop_;
    const int fd_;
    int events_;  //注册fd感兴趣的事件
    int revents_; // poller 返回的事件，是读取还是关闭等事件，调用相对应的回调
    int index_;   //代表此Channel的三种状态 kNew 未添加入epoll模型 kAdded 已添加入epoll模型 kDeleted 已从epoll模型中删除

    std::weak_ptr<void> tie_; //使用weak_ptr跨线程监听Cannel状态
    bool tied_;

    // Channel通过revents 可以得知发生的事件，调用相对应的回调
    ReadEventCallBack readCallBack_;
    EventCallBack writeCallBack_;
    EventCallBack closeCallBack_;
    EventCallBack errorCallBack_;
};
