#ifndef GAME_MSG_H
#define GAME_MSG_H
#include "../common/protocol_constants.h"
#include <vector>

class GameMsg {
    private:
    uint8_t type;
    Direction direction;
    std::vector<std::vector<elements>> map;

    public:
    GameMsg(uint8_t type, Direction direction = DIR_NORTH) : type(type), direction(direction) {}
    uint8_t get_type() const;
    void set_direction(Direction dir);
    Direction get_direction() const;
    void set_map(const std::vector<std::vector<elements>>& new_map);
    const std::vector<std::vector<elements>>& get_map() const;
    void set_type(uint8_t new_type);
    std::string get_chat_content();
};


#endif  // GAME_MSG_H   
