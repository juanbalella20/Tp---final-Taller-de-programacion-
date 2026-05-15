#include "serverDeserializer.h"
#include "../common/protocol_constants.h"

void ServerDeserializer::deserialize_register(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    uint8_t name_len = payload[0];
    std::string name(payload.begin() + LEN_NAME_SIZE_FIELD,
                     payload.begin() + LEN_NAME_SIZE_FIELD + name_len); //aca deseraliza todo el nombre
                     // arranca en playlod[1) donde saltea largo nombre,hasta el byte deonde termina el nombre]
    cmd.set_player_name(name);
    cmd.set_race(RACE_MAP_INV.at(payload[LEN_NAME_SIZE_FIELD + name_len]));
    cmd.set_class(CLASS_MAP_INV.at(payload[LEN_NAME_SIZE_FIELD + name_len + LEN_RACE]));
}

void ServerDeserializer::deserialize_move(const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    cmd.set_direction(static_cast<Direction>(payload[0]));
}

void ServerDeserializer::deserialize_cmd(uint8_t type, const std::vector<uint8_t>& payload, ClientCmd& cmd) {
    cmd.set_message_type(static_cast<MessageType>(type));

    switch (static_cast<MessageType>(type)) {
        case MSG_REGISTER:
            deserialize_register(payload, cmd);
            break;

        case MSG_MOVE:
            deserialize_move(payload, cmd);
            break;

        case MSG_LOGIN:
        case MSG_ATTACK:
        case MSG_TAKE:
        case MSG_THROW:
        case MSG_EQUIP:
        case MSG_MEDITATE:
        case MSG_RESURRECT:
        case MSG_CURE:
        case MSG_LIST:
        case MSG_BUY:
        case MSG_SELL:
        case MSG_DEPOSIT:
        case MSG_RETIRE:
        case MSG_DEP_GOLD:
        case MSG_RET_GOLD:
        case MSG_PRIVATE:
        case MSG_SELECT:
        case MSG_FOUND_CLAN:
        case MSG_JOIN_CLAN:
        case MSG_REV_CLAN:
        case MSG_CLAN_ACEP:
        case MSG_CLAN_RECH:
        case MSG_CLAN_BAN:
        case MSG_CLAN_KICK:
        case MSG_LEFT_CLAN:
            break;

        default:
            break;
    }
}
