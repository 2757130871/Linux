#include <stdio.h>
#include <string>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

class TcpClient
{
public:
    TcpClient(const string& serv_ip = "127.0.0.1", int port = 8888) : _serv_ip(serv_ip), _serv_port(port)
    {
        
        init();
    }
 
    void init()
    {
        struct sockaddr_in servaddr;

        _sock = socket(AF_INET, SOCK_STREAM, 0);

        memset(&servaddr, '\0', sizeof(struct sockaddr_in));

        servaddr.sin_family = AF_INET;
        inet_pton(AF_INET, _serv_ip.c_str(), &servaddr.sin_addr);
        servaddr.sin_port = htons(_serv_port);

        connect(_sock, (struct sockaddr*)&servaddr, sizeof(servaddr));
    }

    void start()
    {
        char buf[256];
        while (1)
        {
            int rd = read(0, buf, sizeof(buf));
            buf[rd] = 0;
            int wr = write(_sock, buf, strlen(buf));
            printf("send %d bit data\n", wr);

            memset(buf, 0, sizeof(buf));
            rd = read(_sock, buf, sizeof(buf));
            if (rd > 0)
            {
                buf[rd] = 0;
                printf("receive %d bit data : %s\n", rd, buf);
                fflush(stdin);
            }
            else if (rd == 0)
            {
                printf("servers exit \n");
                close(_sock);
                exit(1);
            }
            else {
                printf("unknown error\n");
                exit(-1);
            }

        }//end of while

        printf("finished.\n");
        close(_sock);
    }

private:
    string _serv_ip;
    int _serv_port;
    int _sock;
};


int main(int argc, char* argv[]) {

    if(argc != 3)
    {
        printf("input error\n");
        exit(1);
    }
    string serv_ip = argv[1];
    int serv_port = atoi(argv[2]);
    
    TcpClient client(serv_ip, serv_port);
    client.start();
  
    return 0;
}
