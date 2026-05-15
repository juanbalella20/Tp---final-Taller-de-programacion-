#include "serverDeserializer.h"
#include "../common/protocol_constants.h"

ServerDeserializer::ServerDeserializer() {
    handlers[MSG_REGISTER] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_register(payload, cmd);
    };
    handlers[MSG_MOVE] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_move(payload, cmd);
    };
    handlers[MSG_ATTACK] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_attack(payload, cmd);
    };
    handlers[MSG_BUY] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_buy(payload, cmd);
    };
    handlers[MSG_SELL] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_sell(payload, cmd);
    };
    handlers[MSG_EQUIP] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_equip(payload, cmd);
    };
    handlers[MSG_SELECT] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_msg_select(payload, cmd);
    };
}


void ServerDeserializer::deserialize_register(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    uint8_t name_len = payload[0];
    std::string name(payload.begin() + LEN_NAME_SIZE_FIELD,
                     payload.begin() + LEN_NAME_SIZE_FIELD + name_len);//aca deseraliza todo el nombre
                     // arranca en playlod[1) donde saltea largo nombre,hasta el byte deonde termina el nombre]
    cmd.set_player_name(name);
    cmd.set_race(RACE_MAP_INV.at(payload[LEN_NAME_SIZE_FIELD + name_len]));
    cmd.set_class(CLASS_MAP_INV.at(payload[LEN_NAME_SIZE_FIELD + name_len + LEN_RACE]));
}

void ServerDeserializer::deserialize_move(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    cmd.set_direction(static_cast<Direction>(payload[0]));
}

// payload = tipoEntidad(1) + largoNombre(1) + nombre(N)
void ServerDeserializer::deserialize_attack(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    cmd.set_target_type(static_cast<EntityType>(payload[0]));
    uint8_t target_len = payload[1];
    std::string target_name(payload.begin() + LEN_ENTITY + LEN_NAME_SIZE_FIELD,
                            payload.begin() +  LEN_ENTITY + LEN_NAME_SIZE_FIELD + target_len);
    cmd.set_target_name(target_name);
}
void ServerDeserializer::deserialize_msg_select(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    cmd.set_target_type(static_cast<EntityType>(payload[0]));
    uint8_t target_len = payload[1];
    std::string target_name(payload.begin() + LEN_ENTITY + LEN_NAME_SIZE_FIELD,
                            payload.begin() +  LEN_ENTITY + LEN_NAME_SIZE_FIELD + target_len);
    cmd.set_target_name(target_name);
}
void ServerDeserializer::deserialize_cmd(uint8_t type, const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    cmd.set_message_type(static_cast<MessageType>(type));
    auto it = handlers.find(type);
    if (it != handlers.end()) {
        it->second(payload, cmd);
    }


}
