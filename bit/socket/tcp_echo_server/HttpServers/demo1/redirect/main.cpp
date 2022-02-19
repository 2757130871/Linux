#include "servers.hpp"



int main(int argc, char *argv[])
{
    if(argc != 2){
        printf("input error\n");
        exit(-1);
    }

    int serv_port = atoi(argv[1]);
    
    HttpServer server(serv_port);
    server.init();
    server.start();
}
