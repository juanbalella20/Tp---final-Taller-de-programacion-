#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <string>

// Lector minimo de la conexion del cliente desde config.toml.
//
// NetworkConfig solo lee la seccion [network] y cae a valores 
// default (localhost:8080) si falta el archivo o la seccion. Asi el cliente
// puede levantar sin depender de la config completa del server.
//
// Formato esperado en config.toml:
//   [network]
//   host = "localhost"
//   port = "8080"
class NetworkConfig {
private:
    // TODO: despues de varios test, sacar estos valores.
    std::string host = "localhost";
    std::string port = "8080";

public:
    NetworkConfig() = default;

    // Carga host/port desde el TOML dado. Si el archivo no existe o no tiene la
    // seccion [network], deja los valores por defecto.
    void load(const std::string& toml_path);

    const std::string& get_host() const;
    const std::string& get_port() const;
};

#endif  // NETWORK_CONFIG_H
