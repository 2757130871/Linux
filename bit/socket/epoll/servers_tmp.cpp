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
#include <sys/epoll.h>


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


	void AddConnection(struct epoll_event* evts)
	{
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		int newfd = accept(_lsock, (struct sockaddr*)&cli_addr, &len);
		check(newfd, "accept");

		struct epoll_event tmp;
		tmp.data.fd = newfd;
		tmp.events = EPOLLIN;
		int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, &tmp);
		if (ret < 0)
		{
			cout << "add link err" << endl;
			exit(-1);
		}

        cout << "add link:" << newfd << endl;
	}

	void ReadMsg(int fd, struct epoll_event* evts) {

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
			int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
			if (ret < 0)
			{
				cout << "delete link err" << endl;
				exit(-1);
			}

			close(fd);
			cout << "client quit :" << fd << endl;
		}
		else {
			cout << "read error" << endl;
			exit(-1);
		}

	}

	void HandlerEvent(struct epoll_event* evts, int n)
	{
		for (int i = 0; i < n; i++)
		{
			if (evts[i].data.fd == _lsock)
				AddConnection(evts);
			else
				ReadMsg(evts[i].data.fd, evts);
		}
	}

	void start()
	{
		epfd = epoll_create(1024);

		struct epoll_event tmp;
		tmp.events = EPOLLIN;
		tmp.data.fd = _lsock;
		int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, _lsock, &tmp);
        check(ret, "ctl");

		struct epoll_event evts[1024];
		while (1)
		{
			int nready;
			switch (nready = epoll_wait(epfd, evts, 1024, 3000))
			{
			case -1:
				cout << "poll error\n";
				break;
			case 0:
				cout << "timeout\n";
				break;
			default:
				HandlerEvent(evts, nready);
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
	//int maxi = -1;	//fd_array µ±ǰخ´𐂱
	int epfd = -1;
	int _lsock = -1;
	int _serv_port = -1;
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
