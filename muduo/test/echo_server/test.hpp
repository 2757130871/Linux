
#include <iostream>
#include <functional>

#include <string>

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

using namespace std::placeholders;

// #include <muduo/net/InetAddress.h>

class CharServer
{
public:
    CharServer(muduo::net::EventLoop *loop,
               const muduo::net::InetAddress &addr,
               const std::string &name) : server_(loop, addr, name), loop_(loop)
    {
        //设置连接和断开连接的处理回调
        server_.setConnectionCallback(std::bind(&CharServer::OnConnection, this, _1));
        //设置接收msg的处理回调
        server_.setMessageCallback(std::bind(&CharServer::OnMessage, this, _1, _2, _3));

        // 1个main reactor 2个sub reactor
        server_.setThreadNum(3);
    }

    void Start()
    {
        server_.start();
    }

    void OnConnection(const muduo::net::TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            std::cout << conn->peerAddress().toIpPort() << std::endl;
            std::cout << "new conn ... " << std::endl;
        }
        else
        {
            std::cout << "conn close ... " << std::endl;
            conn->shutdown();
        }
    }

    void OnMessage(const muduo::net::TcpConnectionPtr &conn,
                   muduo::net::Buffer *buf,
                   muduo::Timestamp ts)
    {
        std::string recv = buf->retrieveAllAsString();
        // conn->send(recv.c_str(), recv.size());
        conn->send(recv);
    }

private:
    muduo::net::TcpServer server_;
    muduo::net::EventLoop *loop_;
};
