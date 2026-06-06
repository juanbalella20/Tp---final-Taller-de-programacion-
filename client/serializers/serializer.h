#ifndef SERIALIZER_H
#define SERIALIZER_H
#include "../common/commands/clientCmd.h"
#include "../common/constants/protocol_constants.h"
#include <vector>
#include <functional>
#include <unordered_map>

class Serializer {
 private:
    using Handler = std::function<std::vector<uint8_t>(const ClientCmd&)>;
    std::unordered_map<uint8_t, Handler> handlers;

    // [REGISTER]
    void write_header(std::vector<uint8_t>& buf, uint8_t type, uint16_t payload_len);
    std::vector<uint8_t> serialize_register(const ClientCmd& cmd);

    // [MOVE]
    std::vector<uint8_t> serialize_move(const ClientCmd& cmd);

    // [ATTACK, SELECT] — x + y coords (uint16_t big endian)
    std::vector<uint8_t> serialize_coords(uint8_t type, const ClientCmd& cmd);

    // entity_type + target_name (ya no usado por ATTACK/SELECT)
    std::vector<uint8_t> serialize_entity_and_name(uint8_t type, const ClientCmd& cmd);

    // [FOUND_CLAN, JOIN_CLAN, CLAN_ACEP, CLAN_RECH, CLAN_BAN, CLAN_KICK] — solo target_name
    std::vector<uint8_t> serialize_name(uint8_t type, const ClientCmd& cmd);

    // [BUY, SELL, EQUIP, THROW, DEPOSIT, RETIRE] — item_id
    std::vector<uint8_t> serialize_item_id(uint8_t type, const ClientCmd& cmd);

    // [MEDITATE, RESURRECT, CURE, LIST, REV_CLAN, LEFT_CLAN]
    std::vector<uint8_t> serialize_no_payload(uint8_t type);

    // [DEP_GOLD, RET_GOLD]
    std::vector<uint8_t> serialize_gold(uint8_t type, const ClientCmd& cmd);

    // [TELEPORT] -> payload [zona:1B]
    std::vector<uint8_t> serialize_zone(uint8_t type, const ClientCmd& cmd);

    std::vector<uint8_t> serialize_private(const ClientCmd& cmd);

 public:
    Serializer();
    std::vector<uint8_t> serialize_cmd(const ClientCmd& cmd);
};

#endif  // SERIALIZER_H
