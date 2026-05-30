#ifndef SERVER_SRC_SERVER_SERIALIZER_H_
#define SERVER_SRC_SERVER_SERIALIZER_H_

#include "../../common/commands/gameMsg.h"
#include <vector>
#include <functional>
#include <unordered_map>

class ServerSerializer {
private:
    using Handler = std::function<std::vector<uint8_t>(const GameMsg&)>;
    std::unordered_map<uint8_t, Handler> handlers;

    void write_header(std::vector<uint8_t>& buf, uint8_t type, uint16_t payload_len);
    std::vector<uint8_t> serialize_move(const GameMsg& msg);
    std::vector<uint8_t> serialize_map(const GameMsg& msg);
    std::vector<uint8_t> serialize_inventory(const GameMsg& msg);
    std::vector<uint8_t> serialize_text(const GameMsg& msg);
    std::vector<uint8_t> serialize_private(const GameMsg& msg);
    std::vector<uint8_t> serialize_npcs_snapshot(const GameMsg& msg);
    std::vector<uint8_t> serialize_items_snapshot(const GameMsg& msg);
    std::vector<uint8_t> serialize_register(const GameMsg& msg);

    std::vector<uint8_t> serialize_value(const GameMsg& msg, uint32_t value);
    std::vector<uint8_t> serialize_gold(const GameMsg& msg);
    std::vector<uint8_t> serialize_mana(const GameMsg& msg);
    std::vector<uint8_t> serialize_hp(const GameMsg& msg);
    std::vector<uint8_t> serialize_xp(const GameMsg& msg);
    std::vector<uint8_t> serialize_players_snapshot(const GameMsg& msg);

 public:
    ServerSerializer();
    std::vector<uint8_t> serialize_cmd(const GameMsg& msg);
};

#endif  // SERVER_SRC_SERVER_SERIALIZER_H_
