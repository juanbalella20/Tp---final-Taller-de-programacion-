#include "clientCmd.h"

ClientCmd::ClientCmd():

        message_type(MSG_LOGIN),
        client_id(0),
        player_name(),
        target_type(ENTITY_PLAYER),
        target_name(),
        race("human"),
        klass("wizard"),
        password(),
        direction(DIR_NORTH),
        gold(0),
        coord_x(0),
        coord_y(0),
        chat_content() {}



void ClientCmd::set_message_type(MessageType type) { this->message_type = type; }
MessageType ClientCmd::get_message_type() const { return this->message_type; }

void ClientCmd::set_client_id(uint32_t id) { this->client_id = id; }
uint32_t ClientCmd::get_client_id() const { return this->client_id; }

void ClientCmd::set_player_name(const std::string& name) { this->player_name = name; }
const std::string& ClientCmd::get_player_name() const { return this->player_name; }

void ClientCmd::set_target_type(EntityType type) { this->target_type = type; }
EntityType ClientCmd::get_target_type() const { return this->target_type; }

void ClientCmd::set_target_name(const std::string& name) { this->target_name = name; }
const std::string& ClientCmd::get_target_name() const { return this->target_name; }

void ClientCmd::set_race(const std::string& race_value) { this->race = race_value; }
const std::string& ClientCmd::get_race() const { return this->race; }

void ClientCmd::set_class(const std::string& class_value) { this->klass = class_value; }
const std::string& ClientCmd::get_class() const { return this->klass; }

void ClientCmd::set_password(const std::string& password_value) { this->password = password_value; }
const std::string& ClientCmd::get_password() const { return this->password; }

void ClientCmd::set_direction(Direction dir) { this->direction = dir; }
Direction ClientCmd::get_direction() const { return this->direction; }

void ClientCmd::set_item_id(const std::string& item_id) { this->item_id = item_id; }
const std::string& ClientCmd::get_item_id() const { return this->item_id; }

void ClientCmd::set_gold(uint32_t amount) { this->gold = amount; }
uint32_t ClientCmd::get_gold() const { return this->gold; }

void ClientCmd::set_coord_x(int x) { this->coord_x = x; }
int ClientCmd::get_coord_x() const { return this->coord_x; }

void ClientCmd::set_coord_y(int y) { this->coord_y = y; }
int ClientCmd::get_coord_y() const { return this->coord_y; }

void ClientCmd::set_chat_content(const std::string& content) { this->chat_content = content; }

const std::string& ClientCmd::get_chat_content() const { return this->chat_content; }
