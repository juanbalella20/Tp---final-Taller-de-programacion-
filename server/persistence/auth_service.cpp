#include "auth_service.h"

#include <cstring>

#include "../../common/constants/game_constants.h"

AuthService::AuthService(PlayerPersistence& persistence): persistence(persistence) {}

AuthResult AuthService::try_register(const std::string& name,
                                     const std::string& password) const {
    (void)password;  // el password lo guarda el game loop al crear el record
    AuthResult result;
    if (persistence.has_player(name)) {
        result.status = AuthStatus::NAME_TAKEN;
    } else {
        result.status = AuthStatus::OK_NEW;
    }
    return result;
}

AuthResult AuthService::try_login(const std::string& name,
                                  const std::string& password) const {
    AuthResult result;

    // Mismo error si no existe o si la password no coincide (no filtrar nombres).
    if (!persistence.has_player(name) || !persistence.load(name, result.record)) {
        result.status = AuthStatus::BAD_CREDENTIALS;
        return result;
    }

    std::string stored(result.record.password,
                       ::strnlen(result.record.password, PERSIST_PASSWORD_MAX));
    if (stored != password) {
        result.status = AuthStatus::BAD_CREDENTIALS;
        return result;
    }

    result.status = AuthStatus::OK_EXISTING;  // result.record ya tiene el record cargado
    return result;
}
