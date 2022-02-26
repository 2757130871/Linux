#include "client.hpp"


int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cout << "input error" <<endl;
        exit(1);
    }

    Client* c1 = new Client(argv[1], atoi(argv[2]));
    c1->init();
    c1->service();
}
