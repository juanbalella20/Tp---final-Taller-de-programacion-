#include "serializer.h"
#include "../common/protocol_constants.h"
#include <arpa/inet.h>

std::vector<uint8_t> Serializer::serialize_register(const ClientCmd& cmd) {
    const std::string& name = cmd.get_player_name();
    uint8_t name_len = static_cast<uint8_t>(name.size());
    // payload = largoNombre(1) + nombre(N) + raza(1) + clase(1)
    uint16_t payload_len = 1 + name_len + 1 + 1;

    std::vector<uint8_t> buf;
    buf.reserve(3 + payload_len);

    // header
    buf.push_back(MSG_REGISTER);
    uint16_t largo_be = htons(payload_len);
    buf.push_back((largo_be >> 8) & 0xFF);
    buf.push_back(largo_be & 0xFF);

    // payload
    buf.push_back(name_len);
    for (char c : name) buf.push_back(static_cast<uint8_t>(c));
    buf.push_back(static_cast<uint8_t>(RACE_MAP.at(cmd.get_race())));
    buf.push_back(static_cast<uint8_t>(CLASS_MAP.at(cmd.get_class())));

    return buf;
}

std::vector<uint8_t> Serializer::serialize_cmd(const ClientCmd& cmd) {
    switch (cmd.get_message_type()) {
        case MSG_REGISTER:
            return serialize_register(cmd);

        case MSG_LOGIN:
        case MSG_MOVE:
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
            return {};

        default:
            return {};
    }
}
