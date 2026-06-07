#include "clientApp.h"
#include "../common/constants/network_config.h"

#include <iostream>

// ./client sin argumentos: host/puerto salen de config.toml ([network]).
int main() {
    NetworkConfig net_config;
    net_config.load("config.toml");

    ClientApp client(net_config.get_host(), net_config.get_port());
    client.run();

    return 0;
}
