#pragma once

#include "../base/noncopyable.hpp"
#include "../base/InetAddress.hpp"

#include "CallBacks.h"
#include "Buffer.h"

#include <memory>
#include <atomic>

#include <string>

class EventLoop;
class Socket;
class Channel;
class Timestamp;

// TcpServer > Accetor 获取新连接 > 打包成TcpConnetion 设置回调 >>  Channel >> poller 执行回调
//
//封装Tcp连接 描述已建立的连接之间的关系
class TcpConnection : noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    enum StateE
    {
        kDisConnected, //已断开连接
        kConnecting,   //正在连接
        kConnected,    //已连接
        kDisConnecting //正在断开连接
    };

    TcpConnection(EventLoop *loop,
                  const std::string nameArg,    /**/
                  int sockfd,                   /**/
                  const InetAddress &localAddr, /**/
                  const InetAddress &peerAddr   /**/
    );

    ~TcpConnection();

    EventLoop *GetLoop() const { return loop_; }
    const std::string &Name() const { return name_; }

    const InetAddress &LocalAdress() const { return localAddr_; }
    const InetAddress &PeerAdress() const { return peerAddr_; }

    bool Connectioned() const { return state_ == kConnected; }
    bool DisConnectioned() const { return state_ == kDisConnected; }

    //用户设置TcpServer回调用 >> TcpServer::NewConnection 中设置新连接回调
    void SetConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
    void SetMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
    void SetWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
    void SetCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }
    void SetHighWaterMarkCallback(const HighWaterMarkCallback &cb, size_t highWaterMark)
    {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    //发送数据
    void Send(const void *msg, int len);
    //关闭当前连接
    void Shutdown();

    void ShutdownInLoop();

    //连接调用时掉用  调用channel_->Tie
    void ConnectEstablished();
    //连接销毁时调用
    void ConnectDestroyed();

    void Send(const std::string &buf);

private:
    void HandleRead(TimeStamp receiveTime);
    void HandleWrite();
    void HandleClose();
    void HandleError();

    //设置 fd 状态
    void SetState(StateE state) { state_ = state; };
    void SendInLoop(const void *data, size_t len);

private:
    EventLoop *loop_; //此loop都是sub_loop 不可能是base_loop
    const std::string name_;
    std::atomic_int32_t state_;
    bool reading_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    // TcpServer > TcpConnection > Channel
    ConnectionCallback connectionCallback_;       // 有新连接时的回调
    MessageCallback messageCallback_;             // 读写事件就绪时触发的回调
    WriteCompleteCallback writeCompleteCallback_; // 消息发送完毕后触发的回调
    CloseCallback closeCallback_;

    HighWaterMarkCallback highWaterMarkCallback_;
    size_t highWaterMark_; //高水位标志

    Buffer inputBuffer_;
    Buffer outputBuffer_;
};
