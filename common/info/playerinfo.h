#ifndef PLAYERINFO_H_
#define PLAYERINFO_H_

#include <string>
#include "../../common/constants/game_constants.h"
#include "../../common/constants/protocol_constants.h"


struct PlayerInfo {
    std::string name;
    std::string race;
    uint8_t klass;
    int x;
    int y;
    Direction direction = DIR_SOUTH;
    bool has_equipped_weapon = false;
    bool ghost = false;
    std::vector<std::string> equipped_ids;
    // Nombre del clan al que pertenece el jugador ("" si no tiene clan). Lo usa
    // el cliente para pintar el nombre sobre el jugador: verde si es del mismo
    // clan que el jugador local, rojo si es de otro clan (o sin clan).
    std::string clan_name;
    bool moving = false;
    bool update_frame = true;
};


#endif // PLAYERINFO_H_