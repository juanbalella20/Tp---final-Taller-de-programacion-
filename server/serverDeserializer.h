#ifndef SERVER_DESERIALIZER_H
#define SERVER_DESERIALIZER_H

#include "../common/clientCmd.h"
#include "../common/protocol_constants.h"
#include <vector>
#include <cstdint>
#include <functional>
#include <unordered_map>

class ServerDeserializer {
 private:
    using Handler = std::function<void(const std::vector<uint8_t>&, ClientCmd&)>;
    std::unordered_map<uint8_t, Handler> handlers;

    void read_item_id(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void read_entity_and_name(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void read_gold(const std::vector<uint8_t>& payload, ClientCmd& cmd);

    void deserialize_register(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void deserialize_move(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void read_coords(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void deserialize_attack(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void deserialize_no_payload(const std::vector<uint8_t>& payload, ClientCmd& cmd);

 public:
    ServerDeserializer();
    void deserialize_cmd(uint8_t type, const std::vector<uint8_t>& payload, ClientCmd& cmd);
};

#endif  // SERVER_DESERIALIZER_H
