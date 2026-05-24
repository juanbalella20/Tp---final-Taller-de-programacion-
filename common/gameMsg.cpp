#include "gameMsg.h"

uint8_t GameMsg::get_type() const { return type; }
void GameMsg::set_direction(Direction dir) { this->direction = dir; }
Direction GameMsg::get_direction() const { return direction; }
void GameMsg::set_map(const std::vector<std::vector<elements>>& new_map) { this->map = new_map; }
const std::vector<std::vector<elements>>& GameMsg::get_map() const { return map; }
void GameMsg::set_type(uint8_t new_type) { this->type = new_type; }
void GameMsg::set_chat_content(const std::string& content) { this->chat_content = content; }
const std::string& GameMsg::get_chat_content() const { return this->chat_content; }
void GameMsg::set_item_id(const std::string& id) { this->item_id = id; }
const std::string& GameMsg::get_item_id() const { return this->item_id; }
void GameMsg::set_gold(uint32_t gold) { this->gold = gold; }
uint32_t GameMsg::get_gold() const { return this->gold; }

void GameMsg::set_player_name(const std::string& name) { this->player_name = name; }
const std::string& GameMsg::get_player_name() const { return player_name; }
void GameMsg::set_coord_x(int x) { this->coord_x = x; }
int GameMsg::get_coord_x() const { return coord_x; }
void GameMsg::set_coord_y(int y) { this->coord_y = y; }
int GameMsg::get_coord_y() const { return coord_y; }
void GameMsg::set_items(const std::vector<ItemInfo>& new_items) { this->items = new_items; }
const std::vector<ItemInfo>& GameMsg::get_items() const { return items; }
