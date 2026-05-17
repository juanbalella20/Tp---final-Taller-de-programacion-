#include "deserializer.h"
#include "../common/protocol_constants.h"

#include <stdexcept>

ClientDeserializer::ClientDeserializer() {
    handlers[MSG_MOVE] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_move(payload, msg);
    };
    handlers[MSG_SEND_MAP] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_map(payload, msg);
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

// Formato esperado para MSG_SEND_MAP:
// - El server manda un payload "plano" de HEIGHT * WIDTH bytes.
// - Cada byte representa una celda del mapa codificada como ELEMENT_TYPE.
// - El orden es por filas: primero toda la fila 0, luego fila 1, etc.
//
// Pasos de deserializacion:
// 1. Validar largo exacto del payload para evitar leer fuera de rango.
// 2. Recorrer byte a byte y convertir ELEMENT_TYPE -> elements.
// 3. Reconstruir la matriz 2D [HEIGHT][WIDTH].
// 4. Guardarla en GameMsg con set_map(), para que la UI/logica cliente la use.
void ClientDeserializer::deserialize_map(const std::vector<uint8_t>& payload, GameMsg& msg) {
    const size_t expected_len = static_cast<size_t>(HEIGHT) * static_cast<size_t>(WIDTH);
    if (payload.size() != expected_len) {
        throw std::invalid_argument("Payload invalido para MSG_SEND_MAP");
    }

    std::vector<std::vector<elements>> map(
        HEIGHT, std::vector<elements>(WIDTH, elements::empty));

    size_t index = 0;
    for (int row = 0; row < HEIGHT; ++row) {
        for (int col = 0; col < WIDTH; ++col) {
            const uint8_t cell = payload[index++];
            switch (cell) {
                case ELEMENT_PLAYER:
                    map[row][col] = elements::players;
                    break;
                case ELEMENT_NPC:
                    map[row][col] = elements::npcs;
                    break;
                case ELEMENT_OBJECT:
                    map[row][col] = elements::objects;
                    break;
                case ELEMENT_BUILDING:
                    map[row][col] = elements::buildings;
                    break;
                case ELEMENT_EMPTY:
                    map[row][col] = elements::empty;
                    break;
                default:
                    throw std::invalid_argument("Celda invalida en MSG_SEND_MAP");
            }
        }
    }

    msg.set_map(map);
}
