#include "serverApp.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "game_config.h"
#include "utility/paths.h"
#include "../common/constants/network_config.h"

int main(int argc, char* argv[]) {

    // Semilla del RNG una sola vez al arrancar. Sin esto, std::rand() repite
    // siempre la misma secuencia (daño, esquive, crítico, defensa) en cada
    // ejecución del servidor.
    // paths::config: ~/.config/argentum/config.toml si existe (instalado), si no
    // el config.toml del cwd (desarrollo en el repo).

    const std::string config_path = paths::config("config.toml");
    
    NetworkConfig net_config;
    net_config.load(config_path);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    GameConfig::instance().load(config_path);
    ServerApp server(net_config.get_port());
    server.run();

    return 0;
}
