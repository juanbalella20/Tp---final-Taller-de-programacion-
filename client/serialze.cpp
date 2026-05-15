#include "serializer.h"
#include "../common/protocol_constants.h"
#include <arpa/inet.h>
#include <cstring>

Serializer::Serializer() {
    handlers[MSG_REGISTER] = [this](const ClientCmd& cmd) { return serialize_register(cmd); };
    handlers[MSG_MOVE]     = [this](const ClientCmd& cmd) { return serialize_move(cmd); };
    handlers[MSG_ATTACK]   = [this](const ClientCmd& cmd) { return serialize_attack(cmd); };
    handlers[MSG_SELECT]   = [this](const ClientCmd& cmd) { return serialize_msg_select(cmd); };
    handlers[MSG_BUY]      = [this](const ClientCmd& cmd) { return serialize_buy(cmd); };
}

void Serializer::write_header(std::vector<uint8_t>& buf, uint8_t type, uint16_t payload_len) {
    buf.push_back(type);
    uint16_t largo_be = htons(payload_len);
    uint8_t largo_bytes[sizeof(uint16_t)];
    std::memcpy(largo_bytes, &largo_be, sizeof(uint16_t));
    buf.insert(buf.end(), largo_bytes, largo_bytes + sizeof(uint16_t));
}

std::vector<uint8_t> Serializer::serialize_register(const ClientCmd& cmd) {
    const std::string& name = cmd.get_player_name();
    uint8_t name_len = static_cast<uint8_t>(name.size());
    uint16_t payload_len = LEN_NAME_SIZE_FIELD + name_len + LEN_RACE + LEN_CLASS;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_REGISTER, payload_len);
    buf.push_back(name_len);
    buf.insert(buf.end(), name.begin(), name.end());
    buf.push_back(static_cast<uint8_t>(RACE_MAP.at(cmd.get_race())));
    buf.push_back(static_cast<uint8_t>(CLASS_MAP.at(cmd.get_class())));
    return buf;
}

std::vector<uint8_t> Serializer::serialize_move(const ClientCmd& cmd) {
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + 1);
    write_header(buf, MSG_MOVE, 1);
    buf.push_back(static_cast<uint8_t>(cmd.get_direction()));
    return buf;
}

std::vector<uint8_t> Serializer::serialize_attack(const ClientCmd& cmd) {
    const std::string& target = cmd.get_target_name();
    uint8_t target_len = static_cast<uint8_t>(target.size());
    uint16_t payload_len = LEN_ENTITY + LEN_NAME_SIZE_FIELD + target_len;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_ATTACK, payload_len);
    buf.push_back(static_cast<uint8_t>(cmd.get_target_type()));
    buf.push_back(target_len);
    buf.insert(buf.end(), target.begin(), target.end());
    return buf;
}

std::vector<uint8_t> Serializer::serialize_msg_select(const ClientCmd& cmd) {
    const std::string& target = cmd.get_target_name();
    uint8_t target_len = static_cast<uint8_t>(target.size());
    uint16_t payload_len = LEN_ENTITY + LEN_NAME_SIZE_FIELD + target_len;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_SELECT, payload_len);
    buf.push_back(static_cast<uint8_t>(cmd.get_target_type()));
    buf.push_back(target_len);
    buf.insert(buf.end(), target.begin(), target.end());
    return buf;
}

std::vector<uint8_t> Serializer::serialize_buy(const ClientCmd& cmd) {
    const std::string& item_id = cmd.get_item_id();
    uint8_t item_id_len = static_cast<uint8_t>(item_id.size());
    uint16_t payload_len = LEN_NAME_SIZE_FIELD + item_id_len;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_BUY, payload_len);
    buf.push_back(item_id_len);
    buf.insert(buf.end(), item_id.begin(), item_id.end());
    return buf;
}

std::vector<uint8_t> Serializer::serialize_cmd(const ClientCmd& cmd) {
    auto it = handlers.find(cmd.get_message_type());
    if (it != handlers.end()) {
        return it->second(cmd);
    }
    return {};
}
