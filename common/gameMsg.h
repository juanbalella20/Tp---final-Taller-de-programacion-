#ifndef GAME_MSG_H
#define GAME_MSG_H
#include "common/protocol_constants.h"

class GameMsg {
    private:
    uint8_t type;
    Direction direction;

    public:
    GameMsg(uint8_t type, Direction direction = DIR_NORTH) : type(type), direction(direction) {}
    uint8_t get_type() const;
    void set_direction(Direction dir);
    Direction get_direction() const;
    void set_type(uint8_t new_type);
};


#endif  // GAME_MSG_H   