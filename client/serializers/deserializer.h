#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include "../common/commands/gameMsg.h"
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

class ClientDeserializer {
 private:
    using Handler = std::function<void(const std::vector<uint8_t>&, GameMsg&)>;
    std::unordered_map<uint8_t, Handler> handlers;

    void deserialize_move(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_map(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_text(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_name(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_update_equip(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_gold(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_hp(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_xp(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_mana(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_item(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_inventory(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_private(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_npcs_snapshot(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_items_snapshot(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_players_snapshot(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_register(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_confirm_session(const std::vector<uint8_t>& payload, GameMsg& msg);
    uint32_t deserialize_value(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_zone(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_attack(const std::vector<uint8_t>& payload, GameMsg& msg);

 public:
    ClientDeserializer();
    void deserialize_cmd(uint8_t type, const std::vector<uint8_t>& payload, GameMsg& msg);
};

#endif  // DESERIALIZER_H
