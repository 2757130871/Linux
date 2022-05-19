#pragma once

#include "noncopyable.h"
#include "Socket.h"
#include "Channel.h"

#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    using NewConnectionCallBack = std::function<void(int sockfd, const InetAddress &addr)>;

    Acceptor(EventLoop *loop, const InetAddress &listenaddr, bool reuseport = false);
    ~Acceptor();

    void SetNewConnectionCallBack(const NewConnectionCallBack &cb) { newConnectionCallBack_ = cb; }

    //是否为监听状态
    bool Listenning() const { return listenning_; }

    void Listen();

private:
    void HandleRead();

private:
    EventLoop *loop_; // Acceptor运行在main_loop
    Socket acceptSocket_;
    Channel acceptChannel_;

    NewConnectionCallBack newConnectionCallBack_;

    bool listenning_;
};