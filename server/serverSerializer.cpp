#include "serverSerializer.h"
#include "../common/protocol_constants.h"
#include "../common/item_info.h"
#include <arpa/inet.h>
#include <cstring>

ServerSerializer::ServerSerializer() {
    handlers[MSG_MOVE] = [this](const GameMsg& msg) { return serialize_move(msg); };
    handlers[MSG_SEND_MAP] = [this](const GameMsg& msg) { return serialize_map(msg); };
    handlers[MSG_INVENTORY] = [this](const GameMsg& msg) { return serialize_inventory(msg); };
    for (uint8_t type : {
    (uint8_t)MSG_MEDITATE, (uint8_t)MSG_RESURRECT, (uint8_t)MSG_CURE, (uint8_t)MSG_LIST,
    (uint8_t)MSG_FOUND_CLAN, (uint8_t)MSG_JOIN_CLAN, (uint8_t)MSG_LEFT_CLAN, (uint8_t)MSG_CLAN_ACEP,
    (uint8_t)MSG_CLAN_BAN, (uint8_t)MSG_CLAN_KICK, (uint8_t)MSG_CLAN_RECH, (uint8_t)MSG_REV_CLAN,
    (uint8_t)MSG_CHAT,
    }) {
        handlers[type] = [this](const GameMsg& msg) { return serialize_text(msg); };
    };
    for (uint8_t type : {
        (uint8_t)MSG_CHEAT_KILL, (uint8_t)MSG_CHEAT_INF_HP, (uint8_t)MSG_CHEAT_INF_MANA
    }) {
        handlers[type] = [this](const GameMsg& msg) { return serialize_text(msg); };
    };
    handlers[MSG_PRIVATE] = [this](const GameMsg& msg) { return serialize_private(msg); };
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
        payload_len += static_cast<uint16_t>(item.get_id().size()) + 1
                     + static_cast<uint16_t>(item.get_name().size()) + 1;
    }

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_INVENTORY, payload_len);

    for (const auto& item : items) {
        const std::string& id = item.get_id();
        buf.push_back(static_cast<uint8_t>(id.size()));
        buf.insert(buf.end(), id.begin(), id.end());
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

std::vector<uint8_t> ServerSerializer::serialize_text(const GameMsg& msg) {
    const std::string& content = msg.get_chat_content();
    uint16_t payload_len = LEN_NAME_SIZE_FIELD + static_cast<uint16_t>(content.size());

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, static_cast<uint8_t>(msg.get_type()), payload_len);
    buf.push_back(static_cast<uint8_t>(content.size()));
    buf.insert(buf.end(), content.begin(), content.end());
    return buf;
}

std::vector<uint8_t> ServerSerializer::serialize_private(const GameMsg& msg) {
    const std::string& sender = msg.get_player_name();
    const std::string& content = msg.get_chat_content();
    uint16_t payload_len = 1 + sender.size() + 1 + content.size();
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_PRIVATE, payload_len);
    buf.push_back(static_cast<uint8_t>(sender.size()));
    buf.insert(buf.end(), sender.begin(), sender.end());
    buf.push_back(static_cast<uint8_t>(content.size()));
    buf.insert(buf.end(), content.begin(), content.end());
    return buf;
}

