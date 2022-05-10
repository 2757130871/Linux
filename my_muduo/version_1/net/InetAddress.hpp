#pragma once

#include <stdint.h>
#include <string>

#include <cstring>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../base/copyable.hpp"

//封装了sockaddr_in
class InetAddress : copyable
{
public:
    explicit InetAddress(uint16_t port, std::string ip = "127.0.0.1")
    {
        memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        addr_.sin_addr.s_addr = inet_addr(ip.c_str());
    }

    explicit InetAddress(const sockaddr_in &addr)
    {
        addr_ = addr;
    }

    std::string ToIp() const
    {
        return inet_ntoa(addr_.sin_addr);
    }

    std::string ToIpPort() const
    {
        std::string ret;

        ret += inet_ntoa(addr_.sin_addr);
        ret += " ";
        ret += std::to_string(ntohs(addr_.sin_port));
        return ret;
    }

    std::string ToPort() const
    {
        return std::to_string(ntohs(addr_.sin_port));
    }

    const sockaddr_in *GetSockAddr() const
    {
        return &addr_;
    }

    struct sockaddr_in addr_;
};
