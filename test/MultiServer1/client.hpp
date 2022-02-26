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
#include <pthread.h>
using namespace std;



class Client
{
public:
    Client(string serv_ip = "127.0.0.1", int serv_port = 8888): _ip(serv_ip), _port(serv_port)
    {
    }


    void init()
    {
            
       _sock = socket(AF_INET, SOCK_STREAM, 0); 
       if(_sock < 0){
           cout <<  "socket error\n";
           exit(1);
       } 
       
       sockaddr_in serv_addr;
       serv_addr.sin_addr.s_addr = inet_addr(_ip.c_str());
       serv_addr.sin_family = AF_INET;
       serv_addr.sin_port = htons(_port); 
       
       if(connect(_sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
       {
            cout << "connect error" << endl;
            exit(1);
       }
    
    }


    void service()
    {
        char buf[255];
        while(1)
        {
            sleep(2);
            //int rd = read(0, buf, sizeof(buf));
            int rd = 3;
            string str = to_string(pthread_self());
            //m1emmove(buf, str.c_str(), str.size());
            cout << "str" << str << endl;
            memset(buf, 0, sizeof(buf));
            int st = send(_sock, str.c_str(), str.size(), 0);
            cout << "sent " << st << "bit" << endl; 
            fflush(stdout);

            rd = recv(_sock, buf, sizeof(buf) - 1, 0);
            if(rd > 0)
            {
                buf[rd] = 0;
                cout << "recvice: " << buf << endl;
                
            }
            else if(rd == 0){
                cout << "serser close\n";
                exit(-1);
            }
            else{
                cout << "recv error" << endl;
                exit(-1);
            }
            
        }
    }
    
    
private:
    int _sock = -1;
    string _ip;
    int _port;

};
