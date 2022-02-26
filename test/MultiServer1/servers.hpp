#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <signal.h>
#include <stdio.h>
using namespace std;

class Servers
{
public:
    Servers(int port = 8888): _port(port), _lsock(-1) {} 

    void init()
    {
        signal(SIGCHLD, SIG_IGN);
        _lsock = socket(AF_INET, SOCK_STREAM, 0); 
        check(_lsock, "socket");

        sockaddr_in serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(_port);
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        
        check(bind(_lsock, (sockaddr*)&(serv_addr), sizeof(serv_addr)), "bind");
        check(listen(_lsock, 128), "listen");
    }
    
    void start()
    {
        
        while(1)
        {
            sockaddr_in cli_addr;
            socklen_t cli_len = sizeof(cli_len);
            memset(&cli_addr, 0, sizeof(cli_addr));
            
            int cli_sock = accept(_lsock, (sockaddr*)&(cli_addr), &cli_len);
            check(cli_sock, "accept");
            
            cout << "client ip: " << inet_ntoa(cli_addr.sin_addr) << "port: " << htons(cli_addr.sin_port) << endl;

            if(fork() == 0)
            {
                //child 
                close(_lsock);
                service(cli_sock);
                exit(0);
            }
            else {
                close(cli_sock);
            }
            
        }
    
    }
    
    void service(int cli_sock)
    {
        char buf[255];
        while(1)
        {
            int rd = recv(cli_sock, buf, sizeof(buf) - 1, 0);
            
            
            if(rd > 0){

                printf("cli send %d bit: %s\n",rd , buf);
           
                for(size_t i = 0;i < strlen(buf);i++)
                    if(isalpha(buf[i]))
                        buf[i] = toupper(buf[i]);
            }
            else if(rd == 0){
                cout << "client exit \n";
                exit(0);
            }
            else {
                cout << "recv error\n";
                exit(1);
            }

            send(cli_sock, buf, sizeof(buf) - 1, 0);
        }
    }


    void check(int num,const string& str)
    {
        if(num == -1){
            cout << str << "error" << endl;
            exit(-1);
        }
    }

private:
    int _port;
    int _lsock;
};
