#include "servers.hpp"

int main(int argc, char* argv[])
{
    if(argc != 2){
        printf("Input Error\n");
        exit(1);
    }
   
    Servers s1(atoi(argv[1]));
    s1.init();
    s1.start();
}

