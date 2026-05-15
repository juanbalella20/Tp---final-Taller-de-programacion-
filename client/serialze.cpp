#include "serializer.h"
#include "../common/protocol_constants.h"
#include <arpa/inet.h>
#include <cstring>

//          HEADER                                          PAYLOAD
//(1 TIPO BYTE + 2 BYTES LARGO PAYLOAD )+(1 BYTE LARGO NOMBRE + N BYTES NOMBRE + 1 BYTE RAZA + 1 BYTE CLASE)
std::vector<uint8_t> Serializer::serialize_register(const ClientCmd& cmd) {
    const std::string& name = cmd.get_player_name();
    uint8_t name_len = static_cast<uint8_t>(name.size());
    // payload = largoNombre(1) + nombre(N) + raza(1) + clase(1)
    uint16_t payload_len = LEN_NAME_SIZE_FIELD + name_len + LEN_RACE + LEN_CLASS;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len); //reservamos 3, por el header (1 byte tipo + 2 bytes largo) y 
    //el payload que calculamos arriba

    // header
    buf.push_back(MSG_REGISTER);
    uint16_t largo_be = htons(payload_len); // convierte el largo a big-endian (network order)
    uint8_t largo_bytes[sizeof(uint16_t)];
    std::memcpy(largo_bytes, &largo_be, sizeof(uint16_t)); // copia los 2 bytes ya ordenados para enviarlos
    buf.insert(buf.end(), largo_bytes, largo_bytes + sizeof(uint16_t)); // agrega esos 2 bytes al header

    // payload
    buf.push_back(name_len);
    for (char c : name) buf.push_back(static_cast<uint8_t>(c));
    buf.push_back(static_cast<uint8_t>(RACE_MAP.at(cmd.get_race())));
    buf.push_back(static_cast<uint8_t>(CLASS_MAP.at(cmd.get_class())));

    return buf;
}

std::vector<uint8_t> Serializer::serialize_move(const ClientCmd& cmd) {
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + 1);

    buf.push_back(MSG_MOVE);
    uint16_t largo_be = htons(1);
    uint8_t largo_bytes[sizeof(uint16_t)];
    std::memcpy(largo_bytes, &largo_be, sizeof(uint16_t));
    buf.insert(buf.end(), largo_bytes, largo_bytes + sizeof(uint16_t));

    buf.push_back(static_cast<uint8_t>(cmd.get_direction()));
    return buf;
}

std::vector<uint8_t> Serializer::serialize_cmd(const ClientCmd& cmd) {
    switch (cmd.get_message_type()) {
        case MSG_REGISTER:
            return serialize_register(cmd);

        case MSG_MOVE:
            return serialize_move(cmd);

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
            return {};

        default:
            return {};
    }
}
