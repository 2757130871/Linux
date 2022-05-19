#pragma once

#include "noncopyable.h"
#include "Util.h"

#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>

class InetAddress;

//对socket的封装
class Socket : noncopyable
{
public:
    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    ~Socket();

    int Fd() const { return sockfd_; }
    void BindAddress(const InetAddress &localaddr);
    void Listen();
    int Accept(InetAddress *peeraddr);

    void ShutdownWrite();

    void SetTcpNoDelay(bool on);

    void SetReuseAddr(bool on);
    void SetReusePort(bool on);
    void SetKeepAlive(bool on);

    static sockaddr_in GetLockAddr(int sockfd);

private:
    const int sockfd_;
};