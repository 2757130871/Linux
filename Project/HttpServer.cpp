#include "TcpServer.hpp"
#include "Protocol.hpp"
#include <pthread.h>


#define PORT 8989

class HttpServer
{
private:
	int port;
	TcpServer* tcp_server;
	bool stop;
public:

	//HttpServer(int _port=PORT) :port(_port) ,tcp_server()

	HttpServer(int _port = PORT) : port(_port), tcp_server(nullptr), stop(false) {}

	void InitServer() {
		tcp_server = TcpServer::GetInstance(port);
	}

	void Loop() {
		int listen_sock = tcp_server->Sock();

		while (!stop) {
			struct sockaddr_in peer;
			socklen_t len = sizeof(peer);
			int sock = accept(listen_sock, (struct sockaddr*)&peer, &len);
			if (sock < 0) {
				continue;
			}

			int* _sock = new int(sock);
			pthread_t tid;
			pthread_create(&tid, nullptr, Entrance::HandlerRequest, _sock);
			pthread_detach(tid); 
		}
	}

	~HttpServer() {}
};


