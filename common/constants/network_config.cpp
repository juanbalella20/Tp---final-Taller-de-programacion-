#include "network_config.h"
#include "game_config.h"

#include "toml.hpp"
#include "paths.h"

void NetworkConfig::load(const std::string& path) {
    std::string toml_path = paths::config(path);
    toml::table root;
    try {
        root = toml::parse_file(toml_path);
    } catch (const toml::parse_error&) {
        // Sin archivo o TOML invalido: nos quedamos con los defaults.
        return;
    }

    // value_or sobre el valor actual: si la clave no esta, conserva el default.
    host = root.at_path("network.host").value_or("0");
    port = root.at_path("network.port").value_or("0");
}

const std::string& NetworkConfig::get_host() const {
    return host;
}

const std::string& NetworkConfig::get_port() const {
    return port;
}
