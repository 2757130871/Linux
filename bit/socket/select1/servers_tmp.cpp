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

		for (int i = 0; i < 1024; i++)
			fd_array[i] = -1;

		fd_array[0] = _lsock;
		maxi = 0;
	}


	void ReadMsg(int fd, fd_set* r_set, fd_set* allset)
	{
		char buf[1024];

		int rd = read(fd, buf, sizeof(buf) - 1);
		if (rd > 0)
		{
			buf[rd] = 0;
			cout << "client send: " << buf << endl;
		}
		else if (rd == 0)
		{
			//client quit 
            
            for(int i = 0;i < 1024;i++)
            {
                if(fd_array[i] == fd)
                {
                    fd_array[i] = -1;
                    break;
                }
            }

            FD_CLR(fd, allset);
            close(fd);
			cout << "client quit:" << fd << endl;

		}
		else {
			cout << "read bug\n";
			exit(-1);
		}

	}

	void HandlerEvent(fd_set* r_set, fd_set* allset)
	{
		for (int i = 0; i <= maxi; i++)
		{
			if (fd_array[i] == -1)
				continue;

			if (FD_ISSET(fd_array[i], r_set))
			{
				if (fd_array[i] == _lsock)
				{
					struct sockaddr_in cli_addr;
					socklen_t len = sizeof(cli_addr);

					int newfd = accept(_lsock, (struct sockaddr*)&cli_addr, &len);
					check(newfd, "accept");

					int cur = 0;
					for (; cur < 1024; cur++)	//
						if (fd_array[cur] == -1)
							break;

					if (cur >= 1024) {
						cout << "connection is full\n";
						close(newfd);
					}
					else
					{
						cout << "add fd: " << newfd << endl;
						fd_array[cur] = newfd;

						if (newfd > curMax)
							curMax = newfd;

						if (cur > maxi)
							maxi = cur;

						FD_SET(newfd, allset);
					}

				}
				else {
					ReadMsg(fd_array[i], r_set, allset);
				}
			}

		}
	}

	void start()
	{
		struct timeval timer;
		fd_set r_set, allset;
		FD_ZERO(&r_set);
		FD_ZERO(&allset);

		FD_SET(_lsock, &allset);

		int ready = -1;
		curMax = _lsock;

		while (1)
		{
			timer = { 3, 0 };
			r_set = allset;
			switch (ready = select(curMax + 1, &r_set, nullptr, nullptr, &timer))
			{
			case -1:
				printf("select error\n");
				continue;
				break;
			case 0:
				printf("timeout\n");
				continue;
				break;
			default:
				HandlerEvent(&r_set, &allset);
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
