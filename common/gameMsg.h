#ifndef GAME_MSG_H
#define GAME_MSG_H
#include "../common/protocol_constants.h"
#include <vector>

class GameMsg {
    private:
    uint8_t type;
    Direction direction;
    std::vector<std::vector<elements>> map;
    std::string chat_content;
    std::string item_id;
    uint32_t gold;

    public:
    GameMsg(uint8_t type, Direction direction = DIR_NORTH) : type(type), direction(direction) {}
    uint8_t get_type() const;
    void set_direction(Direction dir);
    Direction get_direction() const;
    void set_map(const std::vector<std::vector<elements>>& new_map);
    const std::vector<std::vector<elements>>& get_map() const;
    void set_type(uint8_t new_type);
    void set_chat_content(const std::string& content);
    const std::string& get_chat_content() const;
    void set_item_id(const std::string& id);
    const std::string& get_item_id() const;
    void set_gold(uint32_t gold);
    uint32_t get_gold() const;
};


#endif  // GAME_MSG_H   
