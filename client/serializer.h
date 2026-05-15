#ifndef SERIALIZER_H
#define SERIALIZER_H
#include "../common/clientCmd.h"
#include "../common/protocol_constants.h"
#include <vector>

class Serializer {
 private:
    std::vector<uint8_t> serialize_register(const ClientCmd& cmd);
    std::vector<uint8_t> serialize_move(const ClientCmd& cmd);

 public:
    std::vector<uint8_t> serialize_cmd(const ClientCmd& cmd);
};

#endif  // SERIALIZER_H