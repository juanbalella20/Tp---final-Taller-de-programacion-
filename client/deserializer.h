#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include "../common/gameMsg.h"
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
    void deserialize_gold(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_item(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_inventory(const std::vector<uint8_t>& payload, GameMsg& msg);
    void deserialize_private(const std::vector<uint8_t>& payload, GameMsg& msg);

 public:
    ClientDeserializer();
    void deserialize_cmd(uint8_t type, const std::vector<uint8_t>& payload, GameMsg& msg);
};

#endif  // DESERIALIZER_H
