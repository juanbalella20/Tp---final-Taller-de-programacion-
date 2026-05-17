#include "gameMsg.h"

uint8_t GameMsg::get_type() const { return type; }
void GameMsg::set_direction(Direction dir) { this->direction = dir; }
Direction GameMsg::get_direction() const { return direction; }
void GameMsg::set_map(const std::vector<std::vector<elements>>& new_map) { this->map = new_map; }
const std::vector<std::vector<elements>>& GameMsg::get_map() const { return map; }
void GameMsg::set_type(uint8_t new_type) { this->type = new_type; }
