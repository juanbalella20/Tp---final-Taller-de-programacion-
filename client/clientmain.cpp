#include "clientApp.h"
#include "../common/constants/network_config.h"
#include "../common/constants/game_config.h"
#include "../common/utility/paths.h"

#include <iostream>

// ./client sin argumentos: host/puerto salen de config.toml ([network]).
int main() {
    // paths::config resuelve a ~/.config/argentum/config.toml si existe (cliente
    // instalado); si no, cae al config.toml del cwd (desarrollo en el repo).
    const std::string config_path = paths::config("config.toml");

    NetworkConfig net_config;
    net_config.load(config_path);

    // Rutas de mapas por zona (zones.<nombre>.map): el cliente las resuelve por
    // GameConfig.
    GameConfig::instance().load(config_path);

    ClientApp client(net_config.get_host(), net_config.get_port());
    client.run();

    return 0;
}
