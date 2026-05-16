#include "serverSerializer.h"
#include "../common/protocol_constants.h"
#include <arpa/inet.h>
#include <cstring>

ServerSerializer::ServerSerializer() {
    handlers[MSG_MOVE] = [this](const GameMsg& msg) { return serialize_move(msg); };
}

void ServerSerializer::write_header(std::vector<uint8_t>& buf, uint8_t type, uint16_t payload_len) {
    buf.push_back(type);
    uint16_t largo_be = htons(payload_len);
    uint8_t largo_bytes[sizeof(uint16_t)];
    std::memcpy(largo_bytes, &largo_be, sizeof(uint16_t));
    buf.insert(buf.end(), largo_bytes, largo_bytes + sizeof(uint16_t));
}

std::vector<uint8_t> ServerSerializer::serialize_move(const GameMsg& msg) {
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + LEN_DIRECTION);
    write_header(buf, MSG_MOVE, LEN_DIRECTION);
    buf.push_back(static_cast<uint8_t>(msg.get_direction()));
    return buf;
}

std::vector<uint8_t> ServerSerializer::serialize_cmd(const GameMsg& msg) {
    auto it = handlers.find(msg.get_type());
    if (it != handlers.end()) {
        return it->second(msg);
    }
    return {};
}