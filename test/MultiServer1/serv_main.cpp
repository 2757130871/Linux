#include "servers.hpp"

int main(int argc, const char* argv[])
{
    if(argc != 2){
        cout << "input error\n";
        exit(1);
    }
    
    Servers* s1 = new Servers(atoi(argv[1]));
    s1->init();
    s1->start();
}
