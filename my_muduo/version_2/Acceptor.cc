#include "Acceptor.h"
#include "InetAddress.h"

#include <unistd.h>

// call socket and nonblock
static int CreateNonblocking()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (sockfd < 0)
    {
        LOG(FATAL) << "CreateNonblocking Error !" << std::endl;
        exit(-1);
    }
    return sockfd;
}

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenaddr, bool reuseport)
    : loop_(loop),                              /**/
      acceptSocket_(CreateNonblocking()),       /* 1. socket */
      acceptChannel_(loop, acceptSocket_.Fd()), /**/
      listenning_(false)                        /**/
{
    acceptSocket_.SetReuseAddr(true);
    acceptSocket_.SetReusePort(true);
    acceptSocket_.BindAddress(listenaddr); // 2. bind

    //设置acceptChanel的Read的回调
    // TcpServet.Start() >> Accept.Lisen()   //3. listen
    acceptChannel_.SetReadCallBack(std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor()
{
    acceptChannel_.DisableAll();
    acceptChannel_.Remove();
}

void Acceptor::Listen()
{
    listenning_ = true;
    acceptSocket_.Listen();
    acceptChannel_.EnableReading(); //向Epoll模型中添加listen fd
}

// listenfd就绪 有新连接到来 调用此回调获取新连接
void Acceptor::HandleRead()
{
    InetAddress peerAddr(0);
    int connfd = acceptSocket_.Accept(&peerAddr);
    if (connfd >= 0)
    {
        if (newConnectionCallBack_)
        {
            //通过轮询算法分发一个新连接给某个subloop
            newConnectionCallBack_(connfd, peerAddr);
        }
        else
        {
            close(connfd);
        }
    }
    else
    {
        LOG(ERROR) << "Accept Error !!" << std::endl;
        if (errno == EMFILE)
        {
            LOG(ERROR) << "Socket fd Reached limit !!" << std::endl;
        }
    }
}