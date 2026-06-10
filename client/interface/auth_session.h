#ifndef AUTH_SESSION_H
#define AUTH_SESSION_H

#include <memory>
#include <string>

#include "clientProtocol.h"

// Sesion de autenticacion compartida entre las pantallas (LoginSignupScreen,
// CharacterCreationScreen) y (ScreenManager, ClientApp).
//
// La conexion al socket esta DIFERIDA: protocol arranca en nullptr y recien
// se crea cuando el usuario confirma login o registro.
struct AuthSession {
    std::string name;
    std::string password;   // en claro hasta enviarlo; descartar tras autenticar
    std::string race;       // elegida por el usuario (registro) o del confirm (login)
    std::string klass;      // idem race
    std::string clan;       // clan propio del jugador ("" si no tiene), del confirm

    // Conexion viva una vez que el usuario intenta autenticarse. Se transfiere
    // (move) a los threads de red al entrar al juego.
    std::unique_ptr<ClientProtocol> protocol;

    bool authenticated = false;
};

#endif  // AUTH_SESSION_H
