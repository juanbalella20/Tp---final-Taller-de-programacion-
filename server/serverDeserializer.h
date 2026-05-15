#ifndef SERVER_DESERIALIZER_H
#define SERVER_DESERIALIZER_H

#include "../common/clientCmd.h"
#include <vector>
#include <cstdint>

class ServerDeserializer {
 private:
    void deserialize_register(const std::vector<uint8_t>& payload, ClientCmd& cmd);

 public:
    void deserialize_cmd(uint8_t type, const std::vector<uint8_t>& payload, ClientCmd& cmd);
};

#endif  // SERVER_DESERIALIZER_H
