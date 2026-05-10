#include "serverApp.h"
#include <iostream>

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: ./server <service_or_port>" << std::endl;
        return 1;
    }
    
    const char* port = argv[1];
    ServerApp server(port);
    server.run();

    return 0;
}
