#include "serializer.h"
#include "../common/protocol_constants.h"
#include <arpa/inet.h>
#include <cstring>

Serializer::Serializer() {
    handlers[MSG_REGISTER] = [this](const ClientCmd& cmd) {
        return serialize_register(cmd);
    };
    handlers[MSG_MOVE] = [this](const ClientCmd& cmd) {
        return serialize_move(cmd);
    };
    handlers[MSG_ATTACK] = [this](const ClientCmd& cmd) {
        return serialize_attack(cmd);
    };
}

//          HEADER                                          PAYLOAD
//(1 TIPO BYTE + 2 BYTES LARGO PAYLOAD )+(1 BYTE LARGO NOMBRE + N BYTES NOMBRE + 1 BYTE RAZA + 1 BYTE CLASE)
std::vector<uint8_t> Serializer::serialize_register(const ClientCmd& cmd) {
    const std::string& name = cmd.get_player_name();
    uint8_t name_len = static_cast<uint8_t>(name.size()); // payload = largoNombre(1) + nombre(N) + raza(1) + clase(1)
    uint16_t payload_len = LEN_NAME_SIZE_FIELD + name_len + LEN_RACE + LEN_CLASS;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len); //reservamos 3, por el header (1 byte tipo + 2 bytes largo) y 
    //el payload que calculamos arriba
    // header
    buf.push_back(MSG_REGISTER);
    uint16_t largo_be = htons(payload_len);
    uint8_t largo_bytes[sizeof(uint16_t)];
    std::memcpy(largo_bytes, &largo_be, sizeof(uint16_t));
    buf.insert(buf.end(), largo_bytes, largo_bytes + sizeof(uint16_t));
    //payload
    buf.push_back(name_len);
    buf.insert(buf.end(), name.begin(), name.end());
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

std::vector<uint8_t> Serializer::serialize_attack(const ClientCmd& cmd) {
    const std::string& target = cmd.get_target_name();
    uint8_t target_len = static_cast<uint8_t>(target.size());
    uint16_t payload_len = LEN_ENTITY + LEN_NAME_SIZE_FIELD + target_len;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);

    buf.push_back(MSG_ATTACK);
    uint16_t largo_be = htons(payload_len);
    uint8_t largo_bytes[sizeof(uint16_t)];
    std::memcpy(largo_bytes, &largo_be, sizeof(uint16_t));
    buf.insert(buf.end(), largo_bytes, largo_bytes + sizeof(uint16_t));

    buf.push_back(static_cast<uint8_t>(cmd.get_target_type()));
    buf.push_back(target_len);
    buf.insert(buf.end(), target.begin(), target.end());

    return buf;
}
//  1 byte de tipo + 1 byte de largo + 1 byte de tipo entidad + 1 byte largo nombre + N  bytes nombre
std::vector<uint8_t> Serializer::serialize_msg_select(const ClientCmd& cmd) {
    const std::string& target = cmd.get_target_name();
    uint8_t target_len = static_cast<uint8_t>(target.size());
    uint16_t payload_len = LEN_ENTITY + LEN_NAME_SIZE_FIELD + target_len;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);

    buf.push_back(MSG_SELECT);
    uint16_t largo_be = htons(payload_len);
    uint8_t largo_bytes[sizeof(uint16_t)];
    std::memcpy(largo_bytes, &largo_be, sizeof(uint16_t));
    buf.insert(buf.end(), largo_bytes, largo_bytes + sizeof(uint16_t));

    buf.push_back(static_cast<uint8_t>(cmd.get_target_type()));
    buf.push_back(target_len);
    buf.insert(buf.end(), target.begin(), target.end());

    return buf;
}
/*
std::vector<uint8_t> Serializer::serialize_buy(const ClientCmd& cmd) {
    const s
  
}
*/
std::vector<uint8_t> Serializer::serialize_cmd(const ClientCmd& cmd) {
    auto it = handlers.find(cmd.get_message_type());
    if (it != handlers.end()) {
        return it->second(cmd);
    }
    return {};
}
