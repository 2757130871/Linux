#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("input error\n");
        exit(1);
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(atoi(argv[1]));

    int ret = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret < 0)
    {
        printf("bind error\n");
        exit(1);
    }

    char buf[64];

    char c = 'a';
    while (1)
    {
      if(c == 'g')
      {
        cout << "client sleep" << endl;

          sleep(11);
      }
        if (c >= 'z')
            c = 'a';
        write(sock, &c, 1);

        int rd = recv(sock, buf, 1, 0);
        buf[rd] = 0;
        cout << "server: " << buf << endl;
        c++;

        sleep(1);
    }
}
