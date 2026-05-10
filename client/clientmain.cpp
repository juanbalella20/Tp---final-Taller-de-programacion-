#include "clientApp.h"
#include <iostream>

int main(int argc, const char* argv[]) {

    if (argc != 3) {
        std::cerr << "Usage: ./client <hostname_or_ip> <service_or_port>" << std::endl;
        return 1;
    }
    
    const char* host = argv[1];
    const char* port = argv[2];
    ClientApp client(host, port);
    client.run();

    return 0;
}
