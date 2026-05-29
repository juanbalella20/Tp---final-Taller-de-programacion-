#ifndef PLAYERINFO_H_
#define PLAYERINFO_H_

#include <string>
#include "../../common/constants/game_constants.h"


struct PlayerInfo {
    std::string name;
    uint8_t race;
    uint8_t klass;
    int x;
    int y;
};


#endif // PLAYERINFO_H_