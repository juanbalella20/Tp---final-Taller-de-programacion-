#include "deserializer.h"
#include "../common/protocol_constants.h"

#include <stdexcept>

ClientDeserializer::ClientDeserializer() {
    handlers[MSG_MOVE] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_move(payload, msg);
    };
}

void ClientDeserializer::deserialize_move(const std::vector<uint8_t>& payload, GameMsg& msg) {
    if (payload.size() != LEN_DIRECTION) {
        throw std::invalid_argument("Payload invalido para MSG_MOVE");
    }
    msg.set_direction(static_cast<Direction>(payload[0]));
}

void ClientDeserializer::deserialize_cmd(uint8_t type, const std::vector<uint8_t>& payload, GameMsg& msg) {
    msg.set_type(type);
    auto it = handlers.find(type);
    if (it != handlers.end()) {
        it->second(payload, msg);
    }
}
