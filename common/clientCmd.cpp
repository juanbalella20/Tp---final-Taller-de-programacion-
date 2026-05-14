#include "clientCmd.h"

ClientCmd::ClientCmd():

        message_type(MSG_LOGIN),
        client_id(0),
        player_name(),
        target_type(ENTITY_PLAYER),
        target_name(),
        race(RACE_HUMAN),
        klass(MAGE) {}



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

void ClientCmd::set_race(Race race_value) { this->race = race_value; }
Race ClientCmd::get_race() const { return this->race; }

void ClientCmd::set_class(Class class_value) { this->klass = class_value; }
Class ClientCmd::get_class() const { return this->klass; }
