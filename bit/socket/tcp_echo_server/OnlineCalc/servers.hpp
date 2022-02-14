#include <string.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "data.hpp"


using namespace std;

#define SERV_IP "127.0.0.1"


class Servers
{
public:
    Servers(int port): _port(port), _lsock(-1) {} 
    ~Servers(){
        close(_lsock);
    }
    
    
    void init()
    {
        _lsock = socket(AF_INET, SOCK_STREAM, 0);
        check(_lsock);
        
        struct sockaddr_in serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons(_port);
        serv.sin_addr.s_addr = INADDR_ANY;
        
        check(bind(_lsock, (struct sockaddr*)&serv, sizeof(serv)));
        check(listen(_lsock, 128));

    }

    void start()
    {
        while(true)
        {
            struct sockaddr_in cli_addr;
            socklen_t cli_len = sizeof(cli_addr);
            memset(&cli_addr, 0, sizeof(cli_addr));
            int cli_sock = accept(_lsock, (struct sockaddr*)&cli_addr, &cli_len);
        check(cli_sock);
        
        cout << "client ip: " << inet_ntoa(cli_addr.sin_addr) << "client port: " << ntohs(cli_addr.sin_port) << endl;
        
        service(cli_sock);
        }
    }

    void service(int cli_sock)
    {
        check(cli_sock);
        struct Request_t req;
        struct Response_t  res;
        res.flag = 1;

        recv(cli_sock, &(req), sizeof(req), 0);
        
        switch(req.op){
            case '+':
                res.result = req.x + req.y;
                break;
            case '-':
                res.result = req.x - req.y;
                break;
            case '*':
                res.result = req.x * req.y;
                break;
            case '/':
                if(req.y == 0){
                    res.flag = -1;
                }
                else{
                    res.result = req.x / req.y;
                }
                break;
            default:
                res.flag = -1;
                break;
        }
    
        send(cli_sock, &(res), sizeof(res), 0); 
        close(cli_sock);
    }

    void check(int code)
    {
        if(code < 0){
            printf("error \n");
            exit(1);
        }
    }

private:
    int _port;
    int _lsock;
};
