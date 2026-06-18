#ifndef AUTH_SESSION_H
#define AUTH_SESSION_H

#include <memory>
#include <string>

#include "clientProtocol.h"

struct AuthSession {
    std::string name;
    std::string password; 
    std::string race;    
    std::string klass;  
    std::string clan;

    // Conexion viva una vez que el usuario intenta autenticarse. Se transfiere
    // (move) a los threads de red al entrar al juego.
    std::unique_ptr<ClientProtocol> protocol;

    bool authenticated = false;
};

#endif  // AUTH_SESSION_H
