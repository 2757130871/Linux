
#include "Socket.h"
#include "InetAddress.h"
#include "Util.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <netinet/tcp.h>

#include <unistd.h>
#include <cstring>

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::BindAddress(const InetAddress &localaddr)
{
    int ret = ::bind(sockfd_, (struct sockaddr *)localaddr.GetSockAddr(), sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        LOG(FATAL) << "Socket::BindAddress bind error !!" << std::endl;
        exit(-1);
    }
}
void Socket::Listen()
{
    if (::listen(sockfd_, 1024) < 0)
    {
        LOG(FATAL) << "Socket::Listen listen error !!" << std::endl;
        exit(-1);
    }
}

int Socket::Accept(InetAddress *peeraddr)
{
    sockaddr_in addr;
    socklen_t len = sizeof addr;
    memset(&addr, 0, sizeof addr);

    //获取的新连接默认为非阻塞
    int connfd = ::accept4(sockfd_, (sockaddr *)&addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);

    if (connfd >= 0)
        peeraddr->SetSockAddr(addr);
    else
        LOG(ERROR) << "Socket::Accept accept -1 !!" << std::endl;

    return connfd;
}

void Socket::ShutdownWrite()
{
    if (::shutdown(sockfd_, SHUT_WR) < 0)
        LOG(ERROR) << "Socket::ShutdownWrite shutdown Error!!" << std::endl;
}

void Socket::SetTcpNoDelay(bool on)
{
    int opt = on ? 1 : 0;
    setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
}

void Socket::SetReuseAddr(bool on)
{
    int opt = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void Socket::SetReusePort(bool on)
{
    int opt = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
}

void Socket::SetKeepAlive(bool on)
{
    int opt = on ? 1 : 0;
    setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
}

sockaddr_in Socket::GetLockAddr(int sockfd)
{
    struct sockaddr_in localaddr;
    memset(&localaddr, 0, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname(sockfd, (struct sockaddr *)&localaddr, &addrlen) < 0)
    {
        LOG(ERROR) << "Socket::GetLocalAddr" << std::endl;
    }
    return localaddr;
}