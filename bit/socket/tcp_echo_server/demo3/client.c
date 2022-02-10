#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_PORT 666
#define SERVER_IP  "127.0.0.1"


int main(int argc, char* argv[]) {

    int sockfd;
    char* message;
    struct sockaddr_in servaddr;
    char buf[64];

    //message = argv[1];

    //printf("message: %s\n", message);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&servaddr, '\0', sizeof(struct sockaddr_in));

    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);
    servaddr.sin_port = htons(SERVER_PORT);

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    while(1)
    {
        int rd = read(0, buf, sizeof(buf));
        buf[rd] = 0;
        int wr = write(sockfd, buf, strlen(buf));
        printf("send %d bit data\n", wr);
        
        memset(buf, 0, sizeof(buf));
        rd =read(sockfd, buf, sizeof(buf));
        if(rd > 0)
        {
            buf[rd] = 0;
            printf("receive %d bit data : %s\n", rd, buf);
            fflush(stdin);
        }
        else if(rd == 0)
        {
            printf("servers exit \n");
            close(sockfd);
            exit(1);
        }
        else{
            printf("unknown error\n");
            exit(-1);
        }
        

    }


    printf("finished.\n");
    close(sockfd);

    return 0;
}
