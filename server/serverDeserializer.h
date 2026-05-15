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

    void deserialize_register(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void deserialize_move(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void deserialize_attack(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void deserialize_buy(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void deserialize_sell(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void deserialize_equip(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void deserialize_meditate(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void deserialize_resurrect(const std::vector<uint8_t>& payload, ClientCmd& cmd);
    void deserialize_msg_select(const std::vector<uint8_t>& payload, ClientCmd& cmd);

 public:
    ServerDeserializer();
    void deserialize_cmd(uint8_t type, const std::vector<uint8_t>& payload, ClientCmd& cmd);
};

#endif  // SERVER_DESERIALIZER_H
