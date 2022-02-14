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
#define SERV_PORT 9999

class Client
{
public:
    Client() {} 
    
    void init()
    {
        _sock = socket(AF_INET, SOCK_STREAM, 0);
        check(_sock);
        
        struct sockaddr_in serv;
        serv.sin_family = AF_INET;
        serv.sin_port = htons(serv_port);
        serv.sin_addr.s_addr = inet_addr(serv_ip.c_str());
        //memset(&cli_addr, 0, sizeof(cli_addr));
            
        connect(_sock, (struct sockaddr*)&serv, sizeof(serv));
        check(_sock);
        cout << "server ip: " << inet_ntoa(serv.sin_addr) << "server port: " << ntohs(serv.sin_port) << endl;
    }

    void start()
    {
        service();
    }

    void service()
    {
        check(_sock);
        struct Request_t req;
        struct Response_t  res;
        res.flag = 1;

        cin >> req.x >> req.y >> req.op;
        send(_sock, &(req), sizeof(req), 0); 
        recv(_sock, &(res), sizeof(res), 0);
        
        if(res.flag < 0){
            cout << "input error" << endl;
        }
        else {
            cout << "result: " << res.result << endl;
        }

        close(_sock);
    }

    void check(int code)
    {
        if(code < 0){
            printf("error \n");
            exit(1);
        }
    }

private:
    string serv_ip = SERV_IP;
    int serv_port = SERV_PORT;
    int _sock;
};
