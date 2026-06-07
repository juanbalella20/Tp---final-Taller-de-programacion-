#ifndef AUTH_SESSION_H
#define AUTH_SESSION_H

#include <memory>
#include <string>

#include "clientProtocol.h"

// Sesion de autenticacion compartida entre las pantallas (LoginSignupScreen,
// CharacterCreationScreen) y el andamiaje que arranca el juego (ScreenManager /
// ClientApp).
//
// CONTRATO (fase 0):
//  - Las pantallas de login/creacion LLENAN este struct: crean el `protocol`
//    en el primer intento de conexion, dejan name/race/klass y, ante un
//    MSG_CONFIRM_SESSION, marcan `authenticated = true`.
//  - ClientApp/ScreenManager lo CONSUME: si `authenticated`, mueve el
//    `protocol` hacia los threads de red y construye ClientGUI con name/race.
//
// La conexion al socket esta DIFERIDA: `protocol` arranca en nullptr y recien
// se crea cuando el usuario confirma login o registro.
struct AuthSession {
    std::string name;
    std::string password;   // en claro hasta enviarlo; descartar tras autenticar
    std::string race;       // elegida por el usuario (registro) o del confirm (login)
    std::string klass;      // idem race

    // Conexion viva una vez que el usuario intenta autenticarse. Se transfiere
    // (move) a los threads de red al entrar al juego.
    std::unique_ptr<ClientProtocol> protocol;

    bool authenticated = false;
};

#endif  // AUTH_SESSION_H
