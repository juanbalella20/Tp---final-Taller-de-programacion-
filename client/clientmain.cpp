#include "clientApp.h"
#include <iostream>

int main(int argc, const char* argv[]) {

    if (argc != 6) {
        std::cerr << "Usage: ./client <hostname_or_ip> <service_or_port> <player_name> <race> <klass>" << std::endl;
        return 1;
    }
    
    const char* host = argv[1];
    const char* port = argv[2];
    const char* player_name = argv[3];
    const char* race = argv[4];
    const char* klass = argv[5];
    ClientApp client(host, port, player_name, race, klass);
    client.run();

    return 0;
}
