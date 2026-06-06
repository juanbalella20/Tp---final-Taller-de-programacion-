#include "serverApp.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include "game_config.h" 

int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: ./server <service_or_port>" << std::endl;
        return 1;
    }

    // Semilla del RNG una sola vez al arrancar. Sin esto, std::rand() repite
    // siempre la misma secuencia (daño, esquive, crítico, defensa) en cada
    // ejecución del servidor.
    GameConfig::instance().load("config.toml"); 
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const char* port = argv[1];
    ServerApp server(port);
    server.run();

    return 0;
}
