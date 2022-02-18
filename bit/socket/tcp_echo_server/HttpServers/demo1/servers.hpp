#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>

using namespace std;


class HttpServer
{
public:
    HttpServer(int port): _port(port) ,_lsock(-1) {}
    
    ~HttpServer()
    {
        close(_lsock);
    } 

    void init()
    {
        _lsock = socket(AF_INET, SOCK_STREAM, 0);
        if(_lsock < 0){
            perror("socket error\n");
            exit(-1);
        }
        
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(_port);
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        
        if(bind(_lsock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("bind  error\n");
            exit(-1);
        }
        
        if(listen(_lsock, 128) < 0){
            perror("listen error\n");
            exit(-1);
        }
    }

    void start()
    {
        while(1)
        {
            struct sockaddr_in cli_addr;
            socklen_t cli_len = sizeof(cli_addr);
            
            int cli_sock = accept(_lsock, (struct sockaddr*)&cli_addr, &cli_len);
            if(cli_sock  < 0){
                perror("accept error\n");
                exit(-1);
            }

            service(cli_sock);
        }
    }

    void service(int cli_sock)
    {
        char buf[2500];
        
        int rd = recv(cli_sock, buf, sizeof(buf) - 1, 0);
        buf[rd] = 0;
        printf("%s\n", buf);

        string msg;
        msg += "HTTP/1.0 200 OK\r\n";
        msg += "Content-Type: text/html\r\n";
        msg += "\r\n";
        msg += "<!DOCTYPE html>\
            <html>\
            <head>\
                <title>my title</title>\
                </head>\
                <body>\
                    <p>hello</p>\
                    </body>\
                    </html>";

        send(cli_sock, msg.c_str(), msg.size(), 0);
        close(cli_sock);
    }

private:
    int _port;
    int _lsock;
};


