#include <stdio.h>
#include <string>
#include <arpa/inet.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

class TcpClient
{
public:
	TcpClient(const string& serv_ip = "127.0.0.1", int port = 8888) : _serv_ip(serv_ip), _serv_port(port)
	{
	}

	void init()
	{
		struct sockaddr_in servaddr;

		check(_sock = socket(AF_INET, SOCK_STREAM, 0), "socket");

		memset(&servaddr, '\0', sizeof(struct sockaddr_in));

		servaddr.sin_family = AF_INET;
		inet_pton(AF_INET, _serv_ip.c_str(), &servaddr.sin_addr);
		servaddr.sin_port = htons(_serv_port);

		check(connect(_sock, (struct sockaddr*)&servaddr, sizeof(servaddr)), "connect");
	}

	void check(int num, const string& str)
	{
		if (num < 0)
		{
			cout << str << "error" << endl;
			exit(-1);
		}
	}

	void start()
	{
		char buf[255];
		int rd = read(0, buf, sizeof(buf));

		while (1)
		{
			
			printf("send data,,,,\n");
			int sd = send(_sock, buf, rd, 0);
			sleep(1);
			//rd = read(_sock, buf, sizeof(buf) - 1);
			//buf[rd] = 0;
			//printf("recv %d bit :%s\n",sd, buf);
		}

		printf("finished.\n");
		close(_sock);
	}

private:
	string _serv_ip;
	int _serv_port;
	int _sock;
};

int main(int argc, char* argv[]) {

	if (argc != 3)
	{
		printf("input error\n");
		exit(1);
	}
	string serv_ip = argv[1];
	int serv_port = atoi(argv[2]);

	TcpClient client(serv_ip, serv_port);
	client.init();
	client.start();

	return 0;
}
