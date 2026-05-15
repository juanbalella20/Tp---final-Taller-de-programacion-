#ifndef SERIALIZER_H
#define SERIALIZER_H
#include "../common/clientCmd.h"
#include "../common/protocol_constants.h"
#include <vector>
#include <functional>
#include <unordered_map>

class Serializer {
 private:
    using Handler = std::function<std::vector<uint8_t>(const ClientCmd&)>;
    std::unordered_map<uint8_t, Handler> handlers;

    std::vector<uint8_t> serialize_register(const ClientCmd& cmd);
    std::vector<uint8_t> serialize_move(const ClientCmd& cmd);
    std::vector<uint8_t> serialize_attack(const ClientCmd& cmd);

 public:
    Serializer();
    std::vector<uint8_t> serialize_cmd(const ClientCmd& cmd);
};

#endif  // SERIALIZER_H
