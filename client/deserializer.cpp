#include "deserializer.h"
#include "../common/protocol_constants.h"
#include "../common/game_constants.h"

#include <stdexcept>

ClientDeserializer::ClientDeserializer() {
    handlers[MSG_MOVE] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_move(payload, msg);
    };
    handlers[MSG_SEND_MAP] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_map(payload, msg);
    };
    for (uint8_t type : {
        (uint8_t)MSG_MEDITATE, (uint8_t)MSG_RESURRECT, (uint8_t)MSG_CURE, (uint8_t)MSG_LIST,
        (uint8_t)MSG_FOUND_CLAN, (uint8_t)MSG_JOIN_CLAN, (uint8_t)MSG_LEFT_CLAN, (uint8_t)MSG_CLAN_ACEP,
        (uint8_t)MSG_CLAN_BAN, (uint8_t)MSG_CLAN_KICK, (uint8_t)MSG_CLAN_RECH, (uint8_t)MSG_REV_CLAN
    }) {
        handlers[type] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
            deserialize_text(payload, msg);
        };
    };
    for (uint8_t type : {
        (uint8_t)MSG_BUY, (uint8_t)MSG_SELL, (uint8_t)MSG_DEPOSIT, (uint8_t)MSG_RETIRE, (uint8_t)MSG_TAKE, (uint8_t)MSG_THROW
    }) {
        handlers[type] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
            deserialize_item(payload, msg);
        };
    };
    handlers[MSG_DEP_GOLD] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_gold(payload, msg);
    };
    handlers[MSG_RET_GOLD] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_gold(payload, msg);
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

static std::string read_string(const std::vector<uint8_t>& payload, size_t& offset) {
    if (offset + 1 > payload.size()) {
        throw std::invalid_argument("Payload demasiado corto para leer string");
    }

    uint8_t len = payload[offset];
    offset += 1;

    if (offset + len > payload.size()) {
        throw std::invalid_argument("Payload demasiado corto para leer contenido del string");
    }

    std::string result(payload.begin() + offset, payload.begin() + offset + len);
    offset += len;
    
    return result;
}

void ClientDeserializer::deserialize_text(const std::vector<uint8_t>& payload, GameMsg& msg) {
    size_t offset = 0;
    msg.set_chat_content(read_string(payload, offset));
}

void ClientDeserializer::deserialize_gold(const std::vector<uint8_t>& payload, GameMsg& msg) {
    if (payload.size() != 4) {
        throw std::invalid_argument("Payload inválido para mensaje de oro");
    }

    uint32_t amount = (payload[0] << 24) | (payload[1] << 16) | (payload[2] << 8) | payload[3];
    msg.set_gold(amount);
}

void ClientDeserializer::deserialize_item(const std::vector<uint8_t>& payload, GameMsg& msg) {
    size_t offset = 0;
    msg.set_item_id(read_string(payload, offset));
}
