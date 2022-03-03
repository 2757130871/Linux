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
#include <vector>
#include <iostream>
#include <unordered_map>
#include <sys/poll.h>


using namespace std;


class TcpServers
{
public:

	TcpServers(int port) : _serv_port(port) {}
	~TcpServers() { close(_lsock); }

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

		//set port reuse
		//int opt = 1;
		//setsockopt(_lsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	}


	void AddConnection(struct pollfd* client)
	{
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		int newfd = accept(_lsock, (struct sockaddr*)&cli_addr, &len);
		check(newfd, "accept");

		int i;
		for (i = 0; i < 1024; i++)
		{
			if (client[i].fd == -1)
				break;
		}

		if (i >= 1024) {
			cout << "conection is full" << endl;
			close(newfd);
		}
		else {

			client[i].fd = newfd;
			client[i].events = POLLIN;
			if (i > maxi)
				maxi = i;

			cout << "add client: " << newfd << endl;
		}

	}

	void ReadMsg(int i /* */, struct pollfd* client) {

		char buf[1024];
		int rd = read(client[i].fd, buf, sizeof(buf) - 1);

		if (rd > 0)
		{
			buf[rd] = 0;
			cout << "client send: " << buf << endl;
		}
		else if (rd == 0)
		{
			//client quit
			cout << "client quit :" << client[i].fd << endl;
            close(client[i].fd);
			client[i].fd = -1;
		}
		else {
			cout << "read error" << endl;
			exit(-1);
		}

	}

	void HandlerEvent(struct pollfd* client)
	{
		for (int i = 0; i <= maxi; i++)
		{
			if (client[i].fd == -1)
				continue;

			if (client[i].revents == POLLIN) {

				if (client[i].fd == _lsock)
				{
					AddConnection(client);
				}
				else {
					ReadMsg(i, client);
				}
			}

		}

	}

	void start()
	{
		struct pollfd client[1024];
		client[0].fd = _lsock;
		client[0].events = POLLIN;

		for (int i = 1; i < 1024; i++)				//init -1
			client[i].fd = -1;

		maxi = 0;

		while (1)
		{
			switch (poll(client, 1024, 3000))
			{
			case -1:
				cout << "poll error\n";
				break;
			case 0:
				cout << "timeout\n";
				break;
			default:
				HandlerEvent(client);
				break;
			}
		}
	}

	void check(int num, const string& str)
	{
		if (num < 0)
		{
			cout << "str" << "err" << endl;
			exit(-1);
		}
	}

private:
	int maxi = -1;	//fd_array µ±ǰخ´𐂱
	int curMax = -1;	//µ±ǰخ´򥤴
	int fd_array[1024];
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
	server1.init();
	server1.start();
}
