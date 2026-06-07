#ifndef AUTH_SERVICE_H_
#define AUTH_SERVICE_H_

#include <string>

#include "player_persistence.h"
#include "player_record.h"

// Resultado de un intento de autenticacion.
enum class AuthStatus {
    OK_NEW,           // register valido: el nombre estaba libre, hay que crear el personaje
    OK_EXISTING,      // login valido: credenciales correctas, hay que cargar el record
    NAME_TAKEN,       // register fallido: ya existe un jugador con ese nombre
    BAD_CREDENTIALS,  // login fallido: no existe o la password no coincide
};

struct AuthResult {
    AuthStatus status;
    PlayerRecord record;  // valido solo si status == OK_EXISTING (datos a restaurar)
};

// Resuelve register/login contra la persistencia. Encapsula la regla de negocio
// de autenticacion (nombre libre para register; nombre+password para login) para
// que el game loop solo orqueste segun el resultado. No toca el mundo ni la red.
class AuthService {
public:
    explicit AuthService(PlayerPersistence& persistence);

    // REGISTER: OK_NEW si el nombre esta libre; NAME_TAKEN si ya existe.
    AuthResult try_register(const std::string& name, const std::string& password) const;

    // LOGIN: OK_EXISTING (con el record cargado) si el nombre existe y la password
    // coincide; BAD_CREDENTIALS en caso contrario (mismo mensaje si no existe o si
    // la password es incorrecta, para no filtrar que nombres existen).
    AuthResult try_login(const std::string& name, const std::string& password) const;

private:
    PlayerPersistence& persistence;
};

#endif  // AUTH_SERVICE_H_
