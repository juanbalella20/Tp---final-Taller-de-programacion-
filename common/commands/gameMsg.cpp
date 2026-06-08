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
void GameMsg::set_npcs(const std::vector<NpcInfo>& new_npcs) { this->npcs = new_npcs; }
const std::vector<NpcInfo>& GameMsg::get_npcs() const { return npcs; }
void GameMsg::set_items_on_floor(const std::vector<ItemFloorInfo>& new_items) { this->items_on_floor = new_items; }
const std::vector<ItemFloorInfo>& GameMsg::get_items_on_floor() const { return items_on_floor; }

void GameMsg::set_hp(uint32_t hp) { this->hp = hp; }
uint32_t GameMsg::get_hp() const { return this->hp; }

void GameMsg::set_xp(uint32_t xp) { this->xp = xp; }
uint32_t GameMsg::get_xp() const { return this->xp; }

void GameMsg::set_mana(uint32_t mana) { this->mana = mana; }
uint32_t GameMsg::get_mana() const { return this->mana; }

void GameMsg::set_players(const std::vector<PlayerInfo>& new_players) { this->players = new_players; }
const std::vector<PlayerInfo>& GameMsg::get_players() const { return players; }
void GameMsg::set_player(const PlayerInfo& player) { this->players = {player}; }
void GameMsg::set_zone(Zone z) { this->zone = z;}
Zone GameMsg::get_zone() const { return this->zone; }

void GameMsg::set_race(const std::string& race) { this->race = race; }
const std::string& GameMsg::get_race() const { return this->race; }

void GameMsg::set_class(const std::string& klass) { this->klass = klass; }
const std::string& GameMsg::get_class() const { return this->klass; }

void GameMsg::set_damage(int dmg) { this->damage = dmg; }
int GameMsg::get_damage() const { return this->damage; }

void GameMsg::set_equipped(bool value) { this->equipped = value; }
bool GameMsg::get_equipped() const { return this->equipped; }

void GameMsg::set_equipped_ids(const std::vector<std::string>& ids) { this->equipped_ids = ids; }
const std::vector<std::string>& GameMsg::get_equipped_ids() const { return this->equipped_ids; }

void GameMsg::set_equipped_uids(const std::vector<std::string>& uids) { this->equipped_uids = uids; }
const std::vector<std::string>& GameMsg::get_equipped_uids() const { return this->equipped_uids; }

void GameMsg::set_level(int level) { this->level = level; }
int GameMsg::get_level() const { return this->level; }
