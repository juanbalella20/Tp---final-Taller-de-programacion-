#include "serverDeserializer.h"
#include "../../common/constants/protocol_constants.h"
#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>

ServerDeserializer::ServerDeserializer() {
    handlers[MSG_REGISTER] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_register(payload, cmd);
    };
    handlers[MSG_MOVE] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_move(payload, cmd);
    };
    handlers[MSG_ATTACK] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        read_coords(payload, cmd);
    };
    handlers[MSG_SELECT] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        read_coords(payload, cmd);
    };
    handlers[MSG_BUY]     = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) { read_item_id(payload, cmd); };
    handlers[MSG_SELL]    = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) { read_item_id(payload, cmd); };
    handlers[MSG_EQUIP]   = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) { read_item_id(payload, cmd); };
    handlers[MSG_TAKE]   = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) { read_item_id(payload, cmd); };
    handlers[MSG_THROW]   = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) { read_item_id(payload, cmd); };
    handlers[MSG_DEPOSIT]  = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) { read_item_id(payload, cmd); };
    handlers[MSG_RETIRE]   = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) { read_item_id(payload, cmd); };
    handlers[MSG_DEP_GOLD] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) { read_gold(payload, cmd); };
    handlers[MSG_RET_GOLD] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) { read_gold(payload, cmd); };
    handlers[MSG_GOLD] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) { read_gold(payload, cmd); };
    handlers[MSG_MEDITATE] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_no_payload(payload, cmd);
    };
    handlers[MSG_RESURRECT]= [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_no_payload(payload, cmd);
    };
    handlers[MSG_CURE]     = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_no_payload(payload, cmd);
    };
    handlers[MSG_LIST]     = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_no_payload(payload, cmd);
    };
    handlers[MSG_REV_CLAN] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_no_payload(payload, cmd);
    };
    handlers[MSG_LEFT_CLAN] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_no_payload(payload, cmd);
    };
    handlers[MSG_CHEAT_KILL] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_no_payload(payload, cmd);
    };
    handlers[MSG_CHEAT_INF_HP] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_no_payload(payload, cmd);
    };
    handlers[MSG_CHEAT_INF_MANA] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_no_payload(payload, cmd);
    };
    handlers[MSG_PRIVATE] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_private(payload, cmd);
    };
    handlers[MSG_TELEPORT] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_no_payload(payload, cmd);
    };
    handlers[MSG_FOUND_CLAN] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_clan(payload, cmd);
    };
    handlers[MSG_JOIN_CLAN] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_clan(payload, cmd);
    };
    handlers[MSG_CLAN_ACEP] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_clan(payload, cmd);
    };
    handlers[MSG_CLAN_RECH] = [this](const std::vector<uint8_t>& payload, ClientCmd& cmd) {
        deserialize_clan(payload, cmd);
    };
}


void ServerDeserializer::deserialize_register(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    uint8_t name_len = payload[0];
    //payload+1, es el inicio del nombre; y termina en el inicio del payload + 1 (largo del nombre) + largo del nombre
    std::string name(payload.begin() + LEN_NAME_SIZE_FIELD,
                     payload.begin() + LEN_NAME_SIZE_FIELD + name_len);
    cmd.set_player_name(name);
    cmd.set_race(RACE_MAP_INV.at(payload[LEN_NAME_SIZE_FIELD + name_len]));
    cmd.set_class(CLASS_MAP_INV.at(payload[LEN_NAME_SIZE_FIELD + name_len + LEN_RACE]));
}

void ServerDeserializer::deserialize_move(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    cmd.set_direction(static_cast<Direction>(payload[0]));
}

void ServerDeserializer::read_item_id(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    uint8_t item_len = payload[0];
    std::string item_id(payload.begin() + LEN_NAME_SIZE_FIELD,
                        payload.begin() + LEN_NAME_SIZE_FIELD + item_len);
    cmd.set_item_id(item_id);
}

void ServerDeserializer::read_entity_and_name(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    cmd.set_target_type(static_cast<EntityType>(payload[0]));
    uint8_t target_len = payload[1];
    std::string target_name(payload.begin() + LEN_ENTITY + LEN_NAME_SIZE_FIELD,
                            payload.begin() + LEN_ENTITY + LEN_NAME_SIZE_FIELD + target_len);
    cmd.set_target_name(target_name);
}

void ServerDeserializer::read_coords(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    uint16_t x_be, y_be;
    std::memcpy(&x_be, payload.data(), LEN_COORD);
    std::memcpy(&y_be, payload.data() + LEN_COORD, LEN_COORD);
    cmd.set_coord_x(ntohs(x_be));
    cmd.set_coord_y(ntohs(y_be));
}

void ServerDeserializer::deserialize_attack(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    read_entity_and_name(payload, cmd);
}

void ServerDeserializer::deserialize_no_payload(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    (void)cmd;
    if (!payload.empty()) {
        throw std::invalid_argument("Payload invalido para mensaje sin datos");
    }
}

void ServerDeserializer::deserialize_private(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    size_t offset = 0;
    uint8_t target_len = payload[offset++];
    std::string target(payload.begin() + offset, payload.begin() + offset + target_len);
    offset += target_len;
    uint8_t content_len = payload[offset++];
    std::string content(payload.begin() + offset, payload.begin() + offset + content_len);
    cmd.set_target_name(target);
    cmd.set_chat_content(content);
}

void ServerDeserializer::read_gold(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    uint32_t amount_be;
    memcpy(&amount_be, payload.data(), sizeof(uint32_t));
    cmd.set_gold(ntohl(amount_be));
}

void ServerDeserializer::deserialize_cmd(uint8_t type, const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    cmd.set_message_type(static_cast<MessageType>(type));
    auto it = handlers.find(type);
    if (it != handlers.end()) {
        it->second(payload, cmd);
    }
}

void ServerDeserializer::deserialize_clan(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    uint8_t target_len = payload[0];
    std::string target_name(payload.begin() + LEN_NAME_SIZE_FIELD,
                            payload.begin() + LEN_NAME_SIZE_FIELD + target_len);
    cmd.set_target_name(target_name);
}
