#pragma once
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <iostream>

#include "Util.hpp"

#define BACKLOG 10

//单例模式
class TcpServer
{

public:
	static TcpServer *GetInstance(int port)
	{
		pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
		if (serv == nullptr)
		{
			pthread_mutex_lock(&lock);
			if (serv == nullptr)
			{
				serv = new TcpServer(port);
				serv->Init(); //创建对象直接完成初始化
			}
			pthread_mutex_unlock(&lock);
		}

		return serv;
	}

private:
	TcpServer(int port) : _port(port) {}
	TcpServer(const TcpServer &tcpserver) {} //构造函数全部私有
	~TcpServer()
	{
		if (_lsock > 0)
			close(_lsock);
	}
	void Init()
	{
		Socket();
		Bind();
		Listen();
	}

	void Socket()
	{
		_lsock = socket(AF_INET, SOCK_STREAM, 0);
		if (_lsock < 0)
		{
			LOG(FATAL) <<  "socket error\n";
			exit(-1);
		}

		int opt = 1;
		setsockopt(_lsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	}

	void Bind()
	{
		struct sockaddr_in serv_addr;
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(_port);
		serv_addr.sin_addr.s_addr = INADDR_ANY;

		if (bind(_lsock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			LOG(FATAL) <<  "bind error\n" ;
			exit(-1);
		}
	}

	void Listen()
	{
		if (listen(_lsock, BACKLOG) < 0)
		{
			LOG(FATAL) << "listen error\n"
					   << std::endl;
			exit(-1);
		}
	}

public:
	int Sock()
	{
		return _lsock;
	}

private:
	static TcpServer *serv;
	int _lsock = -1;
	int _port = -1;
};

TcpServer *TcpServer::serv = nullptr;
