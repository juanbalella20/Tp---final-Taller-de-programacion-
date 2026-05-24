#include "serverSerializer.h"
#include "../common/protocol_constants.h"
#include "../common/item_info.h"
#include <arpa/inet.h>
#include <cstring>

ServerSerializer::ServerSerializer() {
    handlers[MSG_MOVE] = [this](const GameMsg& msg) { return serialize_move(msg); };
    handlers[MSG_SEND_MAP] = [this](const GameMsg& msg) { return serialize_map(msg); };
    handlers[MSG_INVENTORY] = [this](const GameMsg& msg) { return serialize_inventory(msg); };
}

void ServerSerializer::write_header(std::vector<uint8_t>& buf, uint8_t type, uint16_t payload_len) {
    buf.push_back(type);
    uint16_t largo_be = htons(payload_len);
    uint8_t largo_bytes[sizeof(uint16_t)];
    std::memcpy(largo_bytes, &largo_be, sizeof(uint16_t));
    buf.insert(buf.end(), largo_bytes, largo_bytes + sizeof(uint16_t));
}

std::vector<uint8_t> ServerSerializer::serialize_inventory(const GameMsg& msg) {
    const std::vector<ItemInfo>& items = msg.get_items();
    uint16_t payload_len = 0;
    for (const auto& item : items) {
        payload_len += LEN_ITEM_ID + static_cast<uint16_t>(item.get_name().size());
    }

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_INVENTORY, payload_len);

    for (const auto& item : items) {
        const std::string& name = item.get_name();
        buf.push_back(static_cast<uint8_t>(name.size()));
        buf.insert(buf.end(), name.begin(), name.end());
    }

    return buf;
}


std::vector<uint8_t> ServerSerializer::serialize_move(const GameMsg& msg) {
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + LEN_DIRECTION);
    write_header(buf, MSG_MOVE, LEN_DIRECTION);
    buf.push_back(static_cast<uint8_t>(msg.get_direction()));
    return buf;
}

std::vector<uint8_t> ServerSerializer::serialize_map(const GameMsg& msg) {
    const auto& map = msg.get_map();
    uint16_t payload_len = 0;
    for (const auto& row : map) {
        payload_len += static_cast<uint16_t>(row.size());
    }

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_SEND_MAP, payload_len);

    for (const auto& row : map) {
        for (elements cell : row) {
            auto it = ELEMENT_TYPE_MAP.find(cell);
            uint8_t encoded = (it != ELEMENT_TYPE_MAP.end()) ? static_cast<uint8_t>(it->second)
                                                              : static_cast<uint8_t>(ELEMENT_EMPTY);
            buf.push_back(encoded);
        }
    }

    return buf;
}

std::vector<uint8_t> ServerSerializer::serialize_cmd(const GameMsg& msg) {
    auto it = handlers.find(msg.get_type());
    if (it != handlers.end()) {
        return it->second(msg);
    }
    return {};
}
