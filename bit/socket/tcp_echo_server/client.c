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
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(atoi(argv[1]));
    
    int ret = connect(listen_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(ret < 0){
        printf("bind error\n");
        exit(1);
    }

    
        char buf[64];

        
        memset(buf, 0, 64);
        size_t rd_count = read(0, buf, 63);
        write(listen_sock, buf, rd_count);
        
        int recv_data = read(listen_sock, buf, 63);
        buf[recv_data] = 0;
        printf("echo data: %s\n", buf);
}
