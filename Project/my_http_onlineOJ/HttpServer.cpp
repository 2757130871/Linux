#include "TcpServer.hpp"
#include "Protocol.hpp"
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include "ThreadPool.hpp"
#include "Task.hpp"

#define PORT 8989

class HttpServer
{
private:
	int port;
	bool stop;

public:
	//HttpServer(int _port=PORT) :port(_port) ,tcp_server()

	HttpServer(int _port = PORT) : port(_port), stop(false) {}

	void InitServer()
	{
		//防止客户端关闭,服务器收到SIGPIPE信号被杀死
		signal(SIGPIPE, SIG_IGN);
	}

	void Loop()
	{
		int listen_sock = TcpServer::GetInstance(port)->Sock();
		ThreadPool *thread_poll = ThreadPool::GetInstance();

		while (!stop)
		{
			struct sockaddr_in peer;
			socklen_t len = sizeof(peer);
			int sock = accept(listen_sock, (struct sockaddr *)&peer, &len);
			if (sock < 0)
			{
				LOG(ERROR) << "accept ERROR !!!\n";
				continue;
			}

			Task *task = new Task(sock);
			//std::cout << "---------- new task: " << task << std::endl;	//DEBUG
			thread_poll->PushTask(task);
		} 

		//销毁单例对象
	}

	~HttpServer()
	{
	}
};
