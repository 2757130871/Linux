#pragma once

#include "noncopyable.h"

//直接包含所需头文件 用户使用TcpServer无需再包含
#include "EventLoop.h"
#include "Acceptor.h"

#include "EventLoopThreadPool.h"
// #include "EventLoopThread.h"
// #include "EventLoop.h"

#include "InetAddress.h"

#include "TcpConnection.h"
#include "CallBacks.h"

#include <string>
#include <memory>
#include <unordered_map>

class TcpServer : noncopyable
{
public:
    using ThreadInitCallBack = std::function<void(EventLoop *)>;

    enum Option
    {
        kNoReusePort,
        kReusePort
    };

    TcpServer(EventLoop *loop, const InetAddress &listenaddr, std::string nameArg, Option opt = kNoReusePort);
    ~TcpServer();

    // TcpServer ==> EventLoopThreadpool SetTheadNum
    void SetThreadNum(int numThreads);

    //  设置thread初始化回调
    void SetTheadInitCallback(const ThreadInitCallBack &cb) { threadInitCallBack_ = cb; }

    //服务器开始监听
    void Start();

    //
    void SetThreadInitCallback(const ThreadInitCallBack &cb) { threadInitCallBack_ = cb; }
    void SetConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void SetMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void SetWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void SetCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

private:
    void NewConnection(int sockfd, const InetAddress &peeraddr);
    void RemoveConnection(const TcpConnectionPtr &conn);
    void RemoveConnectionInLoop(const TcpConnectionPtr &conn);

private:
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop *loop_;          // 用户所定义的EventLoop
    const std::string ipPort_; // ip和port打包成字符串
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_; //运行在mainloop 接收新连接

    std::unique_ptr<EventLoopThreadPool> threadPool_; // one loop per thread

    ConnectionCallback connectionCallback_; // 有新连接时触发的回调
    MessageCallback messageCallback_;       // 读写事件就绪时触发的回调
    CloseCallback closeCallback_;

    WriteCompleteCallback writeCompleteCallback_; //消息发送完毕后触发的回调
    ThreadInitCallBack threadInitCallBack_;       //线程初始化的回调

    int nextConnId_;
    std::atomic_int32_t started_;
    ConnectionMap connections_; //保存所有的连接
};