
#include "TcpServer.h"
#include "TcpConnection.h"

#include <functional>

static EventLoop *CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG(FATAL) << "TcpServer::TcpServer: loop is nullptr !!" << std::endl;
        exit(-1);
    }

    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr, std::string nameArg, Option opt = kNoReusePort)
    : loop_(loop),                                                  /**/
      ipPort_(listenAddr.ToIpPort()),                               /**/
      name_(nameArg),                                               /**/
      acceptor_(new Acceptor(loop, listenAddr, opt == kReusePort)), /**/
      threadPool_(new EventLoopThreadPool(loop, name_)),
      connectionCallback_(),
      messageCallback_(),
      nextConnId_(1),
      started_(0)

{
    //有新连接当来时 Acceptor::HandleRead 中执行TcpServer::NewConnection此回调
    acceptor_->SetNewConnectionCallBack(
        std::bind(TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

// TcpServer  SetTheadNum ==> EventLoopThreadpool SetTheadNum
void TcpServer::SetThreadNum(int numThreads)
{
    threadPool_->SetThreadNum(numThreads);
}

//服务器开始监听 开启线程池和listen_fd
void TcpServer::Start()
{
    //防止被多次Start
    if (started_++ == 0)
    {
        threadPool_->Start(threadInitCallBack_);
        loop_->RunInLoop(std::bind(&Acceptor::Listen, acceptor_.get()));
    }
}

// 有新连接到来 Accepor::HandleRead被回调 通过accept获取一个新连接,
// 并内部执行此回调 选择一个loop 分发connfd
void TcpServer::NewConnection(int sockfd, const InetAddress &peerAddr)
{
    // round robin 获取一个subLoop
    EventLoop *ioLoop = threadPool_->GetNextLoop();

    char buf[64] = {0};
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG(INFO) << "TcpServer::NewConnection" << name_.c_str()
              << "- new connection" << peerAddr.ToIpPort() << std::endl;

    InetAddress localAddr(Socket::GetLockAddr(sockfd));

    //创建一个TcpConnection对象 将连接成功的fd 封装成TcpConnection对象
    TcpConnectionPtr conn(new TcpConnection(
        ioLoop,
        connName,
        sockfd,
        localAddr,
        peerAddr));

    //存入map
    connections_[connName] = conn;

    //用户设置给TcpServer的回调 > TcpConnection > Channel > Poller >> 响应 EventLoop 处理
    conn->SetConnectionCallback(connectionCallback_);
    conn->SetMessageCallback(messageCallback_);
    conn->SetWriteCompleteCallback(writeCompleteCallback_);
    //设置如何关闭连接的回调
    conn->SetCloseCallback(
        std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));

    //直接调用TcpConneciton的ConnectEstablished
    ioLoop->RunInLoop(
        std::bind(&TcpConnection::ConnectEstablished, conn));
}   

void TcpServer::RemoveConnection(const TcpConnectionPtr &conn)
{   
    

}       

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr &conn)
{   


}       
