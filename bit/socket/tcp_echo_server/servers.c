#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if(argc != 2){
        printf("input error\n");
        exit(1);
     }
    
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    int ret = bind(listen_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret < 0){
        printf("bind error\n");
        exit(1);
    }

    if(listen(listen_sock, 128) < 0){
        printf("listen error");
        exit(1);
    }

    char buf[64];
    while(1)
    {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int cli_fd = accept(listen_sock,(struct sockaddr*)&client_addr, &len);

        while(1)
        {

        memset(buf, 0, 64);
        size_t rd_count = recv(cli_fd, buf, 64, 0);
        buf[rd_count] = 0;
        printf("client :%s\n", buf);
        fflush(stdin);
        size_t sd_count = send(cli_fd, buf, rd_count, 0);
        }   
    }

}
