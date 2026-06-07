#include "deserializer.h"
#include "../common/constants/protocol_constants.h"
#include "../common/constants/game_constants.h"
#include "../common/info/item_info.h"
#include "../common/info/playerinfo.h"

#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

ClientDeserializer::ClientDeserializer() {
    handlers[MSG_REGISTER] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_register(payload, msg);
    };
    handlers[MSG_MOVE] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_move(payload, msg);
    };
    handlers[MSG_SEND_MAP] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_map(payload, msg);
    };
    for (uint8_t type : {
        (uint8_t)MSG_MEDITATE, (uint8_t)MSG_RESURRECT, (uint8_t)MSG_CURE, (uint8_t)MSG_LIST,
        (uint8_t)MSG_FOUND_CLAN, (uint8_t)MSG_JOIN_CLAN, (uint8_t)MSG_LEFT_CLAN, (uint8_t)MSG_CLAN_ACEP,
        (uint8_t)MSG_CLAN_BAN, (uint8_t)MSG_CLAN_KICK, (uint8_t)MSG_CLAN_RECH, (uint8_t)MSG_REV_CLAN,
        (uint8_t)MSG_CHAT, (uint8_t)MSG_TAKE
    }) {
        handlers[type] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
            deserialize_text(payload, msg);
        };
    };
    for (uint8_t type : {
        (uint8_t)MSG_BUY, (uint8_t)MSG_SELL, (uint8_t)MSG_DEPOSIT, (uint8_t)MSG_RETIRE
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
    handlers[MSG_GOLD] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_gold(payload, msg);
    };
    
    for (uint8_t type : {
        (uint8_t)MSG_CHEAT_KILL, (uint8_t)MSG_CHEAT_INF_HP, (uint8_t)MSG_CHEAT_INF_MANA
    }) {
        handlers[type] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) { deserialize_text(payload, msg); };
    };
    handlers[MSG_PRIVATE] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_private(payload, msg);
    };
    handlers[MSG_INVENTORY] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_inventory(payload, msg);
    };
    handlers[MSG_NPCS_SNAPSHOT] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_npcs_snapshot(payload, msg);
    };
    handlers[MSG_ITEMS_SNAPSHOT] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_items_snapshot(payload, msg);
    };
    handlers[MSG_HP] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_hp(payload, msg);
    };
    handlers[MSG_XP] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_xp(payload, msg);
    };
    handlers[MSG_MANA] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_mana(payload, msg);
    };
    handlers[MSG_PLAYERS_SNAPSHOT] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_players_snapshot(payload, msg);
    };
    handlers[MSG_ZONE_CHANGE] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_zone(payload, msg);
    };
    handlers[MSG_UPDATE_EQUIP] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_update_equip(payload, msg);
    };
    handlers[MSG_ATTACK] = [this](const std::vector<uint8_t>& payload, GameMsg& msg) {
        deserialize_attack(payload, msg);
    };
}


// Ver formato en serverSerializer::serialize_move:
//   [direction][name_size][name][x BE][y BE]
// Helpers definidos mas abajo en este archivo.
static std::string read_string(const std::vector<uint8_t>& payload, size_t& offset);

// Lee un uint16_t en big-endian del payload
static uint16_t read_uint16_be(const std::vector<uint8_t>& payload, size_t& offset);

// Formato MSG_ZONE_CHANGE: [zona:1B][x:2B BE][y:2B BE]
void ClientDeserializer::deserialize_zone(const std::vector<uint8_t>& payload, GameMsg& msg) {
    if (payload.size() != LEN_ZONE_CHANGE_PAYLOAD) {
        throw std::invalid_argument("Payload invalido para MSG_ZONE_CHANGE");
    }
    msg.set_zone(static_cast<Zone>(payload[0]));
    size_t offset = LEN_ZONE;
    msg.set_coord_x(static_cast<int>(read_uint16_be(payload, offset)));
    msg.set_coord_y(static_cast<int>(read_uint16_be(payload, offset)));
}

void ClientDeserializer::deserialize_attack(const std::vector<uint8_t>& payload, GameMsg& msg) {
    if (payload.size() != LEN_ATTACK_PAYLOAD) {
        throw std::invalid_argument("Payload invalido para MSG_ATTACK");
    }
    size_t offset = 0;
    msg.set_coord_x(static_cast<int>(read_uint16_be(payload, offset)));
    msg.set_coord_y(static_cast<int>(read_uint16_be(payload, offset)));
    msg.set_damage(static_cast<int>(read_uint16_be(payload, offset)));
}

void ClientDeserializer::deserialize_move(const std::vector<uint8_t>& payload, GameMsg& msg) {
    if (payload.size() < LEN_DIRECTION) {
        throw std::invalid_argument("Payload invalido para MSG_MOVE");
    }
    size_t offset = 0;
    msg.set_direction(static_cast<Direction>(payload[offset++]));
    msg.set_player_name(read_string(payload, offset));
    if (offset >= payload.size()) {
        throw std::invalid_argument("Payload demasiado corto leyendo raza de otro jugador en MSG_MOVE");
    }
    uint8_t race_code = payload[offset++];
    std::string race;
    switch (race_code) {
        case RACE_HUMAN: race = "human"; break;
        case RACE_ELF: race = "elf"; break;
        case RACE_DWARF: race = "dwarf"; break;
        case RACE_GNOME: race = "gnome"; break;
        default: throw std::invalid_argument("Raza invalida en MSG_REGISTER");
    }
    msg.set_race(race);
    msg.set_coord_x(static_cast<int>(read_uint16_be(payload, offset)));
    msg.set_coord_y(static_cast<int>(read_uint16_be(payload, offset)));
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

    // La matriz solo lleva terreno (buildings/empty). Actores e items viven en
    // snapshots dedicados (MSG_NPCS_SNAPSHOT / MSG_ITEMS_SNAPSHOT).
    size_t index = 0;
    for (int row = 0; row < HEIGHT; ++row) {
        for (int col = 0; col < WIDTH; ++col) {
            const uint8_t cell = payload[index++];
            switch (cell) {
                case ELEMENT_BUILDING:
                    map[row][col] = elements::buildings;
                    break;
                case ELEMENT_NPC:
                    map[row][col] = elements::npcs;
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
    msg.set_player_name(read_string(payload, offset));
    msg.set_chat_content(read_string(payload, offset));
}

void ClientDeserializer::deserialize_name(const std::vector<uint8_t>& payload, GameMsg& msg) {
    size_t offset = 0;
    msg.set_player_name(read_string(payload, offset));
}

// Formato MSG_UPDATE_EQUIP:
//   [name_size:1B][name][equipped:1B][n_ids:1B] luego n veces: [id_size:1B][id]
void ClientDeserializer::deserialize_update_equip(const std::vector<uint8_t>& payload, GameMsg& msg) {
    size_t offset = 0;
    msg.set_player_name(read_string(payload, offset));
    if (offset < payload.size()) {
        msg.set_equipped(payload[offset] != 0);
        ++offset;
    }
    std::vector<std::string> ids;
    if (offset < payload.size()) {
        uint8_t n_ids = payload[offset];
        ++offset;
        for (uint8_t i = 0; i < n_ids && offset < payload.size(); ++i) {
            ids.push_back(read_string(payload, offset));
        }
    }
    msg.set_equipped_ids(ids);
}

uint32_t ClientDeserializer::deserialize_value(const std::vector<uint8_t>& payload, GameMsg& msg) {
    if (payload.size() != 4) {
        throw std::invalid_argument("Payload inválido para mensaje de oro");
    }

    uint32_t amount;
    std::memcpy(&amount, payload.data(), sizeof(uint32_t));
    uint32_t value = ntohl(amount);

    return value;
}

void ClientDeserializer::deserialize_gold(const std::vector<uint8_t>& payload, GameMsg& msg) {
    uint32_t gold = deserialize_value(payload, msg);
    msg.set_gold(gold);
}

void ClientDeserializer::deserialize_hp(const std::vector<uint8_t>& payload, GameMsg& msg) {
    uint32_t hp = deserialize_value(payload, msg);
    msg.set_hp(hp);
}

void ClientDeserializer::deserialize_xp(const std::vector<uint8_t>& payload, GameMsg& msg) {
    uint32_t xp = deserialize_value(payload, msg);
    msg.set_xp(xp); 
}

void ClientDeserializer::deserialize_mana(const std::vector<uint8_t>& payload, GameMsg& msg) {
    uint32_t mana = deserialize_value(payload, msg);
    msg.set_mana(mana);
}

void ClientDeserializer::deserialize_item(const std::vector<uint8_t>& payload, GameMsg& msg) {
    size_t offset = 0;
    msg.set_item_id(read_string(payload, offset));
}

void ClientDeserializer::deserialize_inventory(const std::vector<uint8_t>& payload, GameMsg& msg) {
    size_t offset = 0;
    std::vector<ItemInfo> items;

    while (offset < payload.size()) {
        std::string id   = read_string(payload, offset);
        std::string name = read_string(payload, offset);
        uint8_t type = payload[offset++];
        items.emplace_back(id, name, 0, type);
    }

    msg.set_items(items);
}
void ClientDeserializer::deserialize_private(const std::vector<uint8_t>& payload, GameMsg& msg) {
    size_t offset = 0;
    msg.set_player_name(read_string(payload, offset));
    msg.set_chat_content(read_string(payload, offset));
}

// Lee un uint16_t en big-endian del payload usando ntohs.
static uint16_t read_uint16_be(const std::vector<uint8_t>& payload, size_t& offset) {
    if (offset + LEN_COORD > payload.size()) {
        throw std::invalid_argument("Payload demasiado corto para leer uint16");
    }
    uint16_t be_value;
    std::memcpy(&be_value, payload.data() + offset, LEN_COORD);
    offset += LEN_COORD;
    return ntohs(be_value);
}

// Ver formato en serializer.
void ClientDeserializer::deserialize_npcs_snapshot(const std::vector<uint8_t>& payload, GameMsg& msg) {
    if (payload.size() < LEN_NPC_COUNT) {
        throw std::invalid_argument("Payload invalido para MSG_NPCS_SNAPSHOT");
    }
    size_t offset = 0;
    uint16_t count = read_uint16_be(payload, offset);

    std::vector<NpcInfo> npcs;
    npcs.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        std::string type = read_string(payload, offset);
        std::string name = read_string(payload, offset);
        int x = static_cast<int>(read_uint16_be(payload, offset));
        int y = static_cast<int>(read_uint16_be(payload, offset));
        npcs.emplace_back(std::move(type), std::move(name), x, y);
    }

    msg.set_npcs(npcs);
}

// Ver formato en serverSerializer::serialize_register.
void ClientDeserializer::deserialize_register(const std::vector<uint8_t>& payload, GameMsg& msg) {
    size_t offset = 0;

    uint16_t rows = read_uint16_be(payload, offset);
    uint16_t cols = read_uint16_be(payload, offset);

    std::vector<std::vector<elements>> map(rows, std::vector<elements>(cols, elements::empty));
    for (uint16_t r = 0; r < rows; ++r) {
        for (uint16_t c = 0; c < cols; ++c) {
            if (offset >= payload.size()) {
                throw std::invalid_argument("Payload demasiado corto leyendo mapa en MSG_REGISTER");
            }
            uint8_t cell = payload[offset++];
            switch (cell) {
                case ELEMENT_BUILDING: map[r][c] = elements::buildings; break;
                case ELEMENT_NPC:      map[r][c] = elements::npcs;      break;
                case ELEMENT_EMPTY:    map[r][c] = elements::empty;     break;
                default: throw std::invalid_argument("Celda invalida en MSG_REGISTER");
            }
        }
    }
    msg.set_map(map);

    if (offset >= payload.size()) {
        throw std::invalid_argument("Payload demasiado corto leyendo item_count en MSG_REGISTER");
    }
    uint8_t item_count = payload[offset++];
    std::vector<ItemInfo> items;
    items.reserve(item_count);
    for (uint8_t i = 0; i < item_count; ++i) {
        std::string id   = read_string(payload, offset);
        std::string name = read_string(payload, offset);
        uint8_t type = payload[offset++];
        items.emplace_back(id, name, 0, type);
    }
    msg.set_items(items);

    uint32_t gold_be, hp_be, xp_be, mana_be, level_be;
    if (offset + 5 * sizeof(uint32_t) > payload.size()) {
        throw std::invalid_argument("Payload demasiado corto leyendo stats en MSG_REGISTER");
    }
    std::memcpy(&gold_be, payload.data() + offset, sizeof(uint32_t)); offset += sizeof(uint32_t);
    std::memcpy(&hp_be,   payload.data() + offset, sizeof(uint32_t)); offset += sizeof(uint32_t);
    std::memcpy(&xp_be,   payload.data() + offset, sizeof(uint32_t)); offset += sizeof(uint32_t);
    std::memcpy(&mana_be, payload.data() + offset, sizeof(uint32_t)); offset += sizeof(uint32_t);
    std::memcpy(&level_be, payload.data() + offset, sizeof(uint32_t)); offset += sizeof(uint32_t);

    msg.set_gold(ntohl(gold_be));
    msg.set_hp(ntohl(hp_be));
    msg.set_xp(ntohl(xp_be));
    msg.set_mana(ntohl(mana_be));
    msg.set_level(static_cast<int>(ntohl(level_be)));

    // Posicion de spawn calculada por el servidor.
    msg.set_coord_x(static_cast<int>(read_uint16_be(payload, offset)));
    msg.set_coord_y(static_cast<int>(read_uint16_be(payload, offset)));

    // Jugadores ya presentes en el mapa al momento del registro.
    uint16_t player_count = read_uint16_be(payload, offset);
    std::vector<PlayerInfo> players;
    players.reserve(player_count);
    for (uint16_t i = 0; i < player_count; ++i) {
        std::string name = read_string(payload, offset);
        if (offset >= payload.size()) {
            throw std::invalid_argument("Payload demasiado corto leyendo raza de otro jugador en MSG_REGISTER");
        }
        uint8_t race_code = payload[offset++];
        int x = static_cast<int>(read_uint16_be(payload, offset));
        int y = static_cast<int>(read_uint16_be(payload, offset));
        std::string race;
        switch (race_code) {
            case RACE_HUMAN: race = "human"; break;
            case RACE_ELF: race = "elf"; break;
            case RACE_DWARF: race = "dwarf"; break;
            case RACE_GNOME: race = "gnome"; break;
            default: throw std::invalid_argument("Raza invalida en MSG_REGISTER");
        }
        players.push_back({std::move(name), race, 0, x, y});
    }
    msg.set_players(players);
}

// Ver formato en serverSerializer::serialize_players_snapshot.
void ClientDeserializer::deserialize_players_snapshot(const std::vector<uint8_t>& payload, GameMsg& msg) {
    if (payload.size() < LEN_PLAYER_COUNT) {
        throw std::invalid_argument("Payload invalido para MSG_PLAYERS_SNAPSHOT");
    }
    size_t offset = 0;
    uint16_t count = read_uint16_be(payload, offset);

    std::vector<PlayerInfo> players;
    players.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        std::string name = read_string(payload, offset);

        if (offset >= payload.size()) {
            throw std::invalid_argument("Payload demasiado corto leyendo raza en MSG_PLAYERS_SNAPSHOT");
        }
        uint8_t race_code = payload[offset++];
        std::string race;
        switch (race_code) {
            case RACE_HUMAN: race = "human"; break;
            case RACE_ELF:   race = "elf";   break;
            case RACE_DWARF: race = "dwarf"; break;
            case RACE_GNOME: race = "gnome"; break;
            default: throw std::invalid_argument("Raza invalida en MSG_PLAYERS_SNAPSHOT");
        }

        int x = static_cast<int>(read_uint16_be(payload, offset));
        int y = static_cast<int>(read_uint16_be(payload, offset));

        if (offset >= payload.size()) {
            throw std::invalid_argument("Payload demasiado corto leyendo ghost en MSG_PLAYERS_SNAPSHOT");
        }

        bool is_ghost = (payload[offset++] != 0);
        PlayerInfo pi{std::move(name), race, 0, x, y};
        pi.ghost = is_ghost;
        players.push_back(pi);
    }

    msg.set_players(players);
}

// Ver formato en serializer.
void ClientDeserializer::deserialize_items_snapshot(const std::vector<uint8_t>& payload, GameMsg& msg) {
    if (payload.size() < LEN_ITEM_COUNT) {
        throw std::invalid_argument("Payload invalido para MSG_ITEMS_SNAPSHOT");
    }
    size_t offset = 0;
    uint16_t count = read_uint16_be(payload, offset);

    std::vector<ItemFloorInfo> items;
    items.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        std::string type = read_string(payload, offset);
        int x = static_cast<int>(read_uint16_be(payload, offset));
        int y = static_cast<int>(read_uint16_be(payload, offset));
        items.emplace_back(std::move(type), x, y);
    }

    msg.set_items_on_floor(items);
}
