#include <pthread.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>

using namespace std;


class TcpServers
{
public:

	TcpServers(int port) : _serv_port(port) {
		init();
	}
	~TcpServers() {
		close(_lsock);
	}

	void init()
	{
        struct sockaddr_in server_addr;
		_lsock = socket(AF_INET, SOCK_STREAM, 0);
		//bzero(&server_addr, sizeof(server_addr));
		server_addr.sin_family = AF_INET;//IPV4
		server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		server_addr.sin_port = htons(_serv_port);

		bind(_lsock, (struct sockaddr*)&server_addr, sizeof(server_addr));

		listen(_lsock, 128);
	}

	void start()
	{
		while (1) {
			struct sockaddr_in client;
			int client_sock, len, i;
			char client_ip[64];
			char buf[256];

			socklen_t  client_addr_len;
			client_addr_len = sizeof(client);
			client_sock = accept(_lsock, (struct sockaddr*)&client, &client_addr_len);

			//打印客服端IP地址和端口号
			printf("client ip: %s\t port : %d\n",
				inet_ntop(AF_INET, &client.sin_addr.s_addr, client_ip, sizeof(client_ip)),
				ntohs(client.sin_port));

			pthread_t t1;
			pthread_create(&t1, NULL, func1, (void*)(long)client_sock);
			pthread_detach(t1);
		}
	}

	static void* func1(void* arg)
	{
		char buf[64];
		int len;
		int client_sock = (int)(long)arg;
		while (1)
		{
			/*  读取客户端发送的数据*/
			len = read(client_sock, buf, sizeof(buf) - 1);
			if (len > 0)
			{
				buf[len] = '\0';
				printf("receive[%d]: %s\n", len, buf);
			}
			else if (len == 0)
			{
				printf("client exit \n");
				break;
			}
			else {
				printf("unknown error\n");
				exit(-1);
			}

			//转换成大写
			int i;
			for (i = 0; i < len; i++) {
				buf[i] = toupper(buf[i]);
			}

			len = write(client_sock, buf, len);
		}

		printf("finished. len: %d\n", len);
		close(client_sock);
		return NULL;
	}

private:
	int _lsock;
	int _serv_port;
};


int main(int argc, char* argv[]) {

	if (argc != 2) {
		printf("input error\n");
		exit(-1);
	}

	int port = atoi(argv[1]);
	TcpServers server1(port);
	server1.start();
}
