#ifndef GAME_MSG_H
#define GAME_MSG_H
#include "protocol_constants.h"
#include "item_info.h"
#include "npc_info.h"
#include "item_floor_info.h"
#include <string>
#include <vector>

class GameMsg {
    private:
    uint8_t type;
    Direction direction;
    std::vector<std::vector<elements>> map;
    std::string chat_content;
    std::string item_id;
    uint32_t gold;
    std::string player_name;
    int coord_x;
    int coord_y;
    std::vector<ItemInfo> items;
    std::vector<NpcInfo> npcs;
    std::vector<ItemFloorInfo> items_on_floor;
    int hp;
    int xp;
    int mana;

    public:
    GameMsg(uint8_t type, Direction direction = DIR_NORTH)
        : type(type), direction(direction), coord_x(0), coord_y(0) {}
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

    void set_player_name(const std::string& name);
    const std::string& get_player_name() const;
    void set_coord_x(int x);
    int get_coord_x() const;
    void set_coord_y(int y);
    int get_coord_y() const;
    void set_items(const std::vector<ItemInfo>& items);
    const std::vector<ItemInfo>& get_items() const;
    void set_npcs(const std::vector<NpcInfo>& npcs);
    const std::vector<NpcInfo>& get_npcs() const;
    void set_items_on_floor(const std::vector<ItemFloorInfo>& items);
    const std::vector<ItemFloorInfo>& get_items_on_floor() const;

    void set_hp(int hp);
    int get_hp() const;

    void set_xp(int xp);
    int get_xp() const;

    void set_mana(int mana);
    int get_mana() const;
};


#endif  // GAME_MSG_H   
