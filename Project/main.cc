#include "HttpServer.cpp"
#include <unistd.h>
#include <memory>

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        std::cout << "./server port" << std::endl;
        exit(-1);
    }

    std::shared_ptr<HttpServer> http_server(new HttpServer(atoi(argv[1])));
    http_server->InitServer();
    http_server->Loop();
}
