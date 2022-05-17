#include "TcpConnection.h"
#include "../base/Util.hpp"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "../base/TimeStamp.hpp"

#include <functional>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h>

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG(FATAL) << "TcpConnection::TcpConnection: loop is nullptr !!" << std::endl;
        exit(-1);
    }

    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop,
                             const std::string nameArg,    /**/
                             int sockfd,                   /**/
                             const InetAddress &localAddr, /**/
                             const InetAddress &peerAddr)
    : loop_(CheckLoopNotNull(loop)),
      name_(nameArg),
      state_(kConnecting), /* 初始化状态为 kConnecting (正在连接) */
      reading_(true),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)), /* 此Channel最终注册到poller上 */
      localAddr_(localAddr),
      peerAddr_(peerAddr),
      highWaterMark_(64 * 1024 * 1024) // 高水位 64M
{

    //给Channel设置相对应的回调
    channel_->SetReadCallBack(
        std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1));

    channel_->SetWriteCallBack(
        std::bind(&TcpConnection::HandleWrite, this));

    channel_->SetCloseCallBack(
        std::bind(&TcpConnection::HandleClose, this));

    channel_->SetErrorCallBack(
        std::bind(&TcpConnection::HandleError, this));

    LOG(INFO) << "TcpConnection:: name: " << name_.c_str() << " sock: " << sockfd << std::endl;

    // 开启 TCP层的keepalive 心跳包
    socket_->SetKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG(INFO) << "TcpConnection:: name: " << name_.c_str() << " sock: " << channel_->Get_Fd() << "state: " << state_ << std::endl;
}

//给Channel绑定的 读事件回调
void TcpConnection::HandleRead(TimeStamp receiveTime)
{
    int saveErrno = 0;
    ssize_t n = inputBuffer_.ReadFd(channel_->Get_Fd(), &saveErrno);

    if (n > 0) //已建立连接的用户，触发了读事件 调用用户设置的回调操作OnMessage
    {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }
    else if (n == 0) //用户断开
    {
        HandleClose();
    }
    else //读取异常
    {
        errno = saveErrno;
        LOG(ERROR) << " TcpConnection::HandleClose: " << strerror(saveErrno) << std::endl;
        HandleError();
    }
}
//给Channel绑定的 写事件回调
void TcpConnection::HandleWrite()
{
    if (channel_->IsWriting())
    {
        int saveErrno = 0;
        ssize_t n = outputBuffer_.WriteFd(channel_->Get_Fd(), &saveErrno);
        if (n > 0)
        {
            outputBuffer_.Retrieve(n);

            //发送缓冲区全部发送完毕
            if (outputBuffer_.ReadableBytes() == 0)
            {
                //发送完毕,取消所有关注的事件
                channel_->DisableWrirting();

                if (writeCompleteCallback_)
                {
                    loop_->QueueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this()));
                }

                if (state_ == kDisConnecting)
                {
                    //把此TcpConnection从当前Loop删除
                    ShutdownInLoop();
                }
            }
        }
        else // n <= 0
        {
            LOG(ERROR) << "TcpConnection::HandleWrite: outputBuffer_.WriteFd Error!" << std::endl;
        }
    }
    else // Channel不可写
    {
        LOG(ERROR) << "TcpConnection::HandleWrite: fd: " << channel_->Get_Fd() << " "
                   << "is down, no more writing" << std::endl;
    }
}

//给Channel绑定的 关闭连接事件回调
void TcpConnection::HandleClose()
{
    LOG(ERROR) << "TcpConnection::HandleClose: fd " << channel_->Get_Fd() << " state: " << state_ << std::endl;
    SetState(kDisConnected);
    channel_->DisableAll();

    TcpConnectionPtr conn(shared_from_this());

    //执行关闭连接的回调
    if (connectionCallback_)
        connectionCallback_(conn);
    //一般用户使用muduo库只需要注册OnMessage和OnConnection
    //商业项目需要更严谨 需要设置 closeCallback_
    if (closeCallback_)
        closeCallback_(conn);
}

//给Channel绑定的 错误事件回调
void TcpConnection::HandleError()
{
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if (::getsockopt(channel_->Get_Fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }

    LOG(ERROR) << "TcpConnection::HandleError: " << name_ << " err: " << err << std::endl;
}

//发送数据
void TcpConnection::Send(const std::string &buf)
{
    if (state_ == kConnected)
    {
        if (loop_->IsInLoopThread()) //处于当前Loop
        {
            SendInLoop(buf.c_str(), buf.size());
        }
        else //不处于当前Loop 唤醒对应Loop执行该回调
        {
            loop_->QueueInLoop(std::bind(
                &TcpConnection::SendInLoop,
                this,
                buf.c_str(),
                buf.size()));
        }
    }
}

//发送数据
//如果应用层发送太快 与内核发送缓冲区发送速度差距太大
//所以需要应用层的发送缓冲区 并设置高水位线 越过水位线触发相对应的回调
void TcpConnection::SendInLoop(const void *data, size_t len)
{
    ssize_t nwrote = 0;      //已发送的数据
    ssize_t remaining = len; //未发送的数据

    bool faultError = false; //是否出错

    //此Connecion已关闭 不能继续发送 直接返回
    if (state_ == kDisConnected)
    {
        LOG(ERROR) << "kDisConnecting, give up writing" << std::endl;
        return;
    }

    //第一次发送数据  此Channel不可写or
    if (!channel_->IsWriting() && outputBuffer_.ReadableBytes() == 0)
    {
        nwrote = ::write(channel_->Get_Fd(), data, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_)
            {
                //数据一次性发送完毕 无需继续给Channel设置EPOLLOUT事件
                loop_->QueueInLoop(
                    std::bind(&TcpConnection::writeCompleteCallback_, shared_from_this()));
            }
        }
        else // nwrote < 0 发送错误
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                //非正常错误
                LOG(ERROR) << "TcpConnection::SendInLoop fd: " << channel_->Get_Fd() << std::endl;

                //写过程中对端断开连接
                if (errno == EPIPE || errno == ECONNRESET)
                {
                    faultError = true;
                }
            }
        }
    }

    //发送正常 并且 此次发送并未发送完毕 需要将数据保存到缓冲区并注册EPOLLOUT事件
    // Poller下次返回此EPOLLOUT事件 调用Channel的WriteCallback 也就是TcpConnection::HandleWrite
    if (faultError != true && remaining > 0)
    {
        size_t oldLen = outputBuffer_.ReadableBytes();
        //是否触发高水位回调
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->QueueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }

        //将未写完的数据写入缓冲区
        outputBuffer_.Append((char *)data + nwrote, remaining);
        if (!channel_->IsWriting())
        {
            //注册此Channel的写事件 下次Poller才会响应此事件
            channel_->EnableWriting();
        }
    }
}

//连接调用时掉用   channel_->Tie
void TcpConnection::ConnectEstablished()
{
    SetState(kConnected); //设置state为已连接
    channel_->Tie(shared_from_this());

    channel_->EnableReading(); //往Epoll模型上注册读事件

    //新连接建立 执行回调
    //此回调调用用户设置的OnConnection回调,连接建立和关闭都会触发
    connectionCallback_(shared_from_this());
}

//连接销毁时调用
void TcpConnection::ConnectDestroyed()
{
    if (state_ == kConnected)
    {
        SetState(kDisConnected);

        //注销此channel关注的所有事件
        channel_->DisableAll();
        connectionCallback_(shared_from_this());
    }

    //将Channel从Poller中移除
    channel_->Remove();
}

//关闭当前连接
void TcpConnection::Shutdown()
{
    if (state_ == kConnected)
    {
        SetState(kDisConnecting);
        loop_->RunInLoop(
            std::bind(&TcpConnection::ShutdownInLoop, this));
    }
}

void TcpConnection::ShutdownInLoop()
{
    //当前发送缓冲区已发送完毕
    if (!channel_->IsWriting())
    {
        socket_->ShutdownWrite(); //关闭写端
    }
}
