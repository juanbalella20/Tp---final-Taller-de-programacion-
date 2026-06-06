#include "serverSerializer.h"
#include "../../common/constants/protocol_constants.h"
#include "../../common/info/item_info.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

static void append_uint16_be(std::vector<uint8_t>& buf, uint16_t value);

ServerSerializer::ServerSerializer() {
    handlers[MSG_REGISTER] = [this](const GameMsg& msg) { return serialize_register(msg); };
    handlers[MSG_MOVE] = [this](const GameMsg& msg) { return serialize_move(msg); };
    handlers[MSG_SEND_MAP] = [this](const GameMsg& msg) { return serialize_map(msg); };
    handlers[MSG_INVENTORY] = [this](const GameMsg& msg) { return serialize_inventory(msg); };
    for (uint8_t type : {
    (uint8_t)MSG_MEDITATE, (uint8_t)MSG_RESURRECT, (uint8_t)MSG_CURE, (uint8_t)MSG_LIST,
    (uint8_t)MSG_FOUND_CLAN, (uint8_t)MSG_JOIN_CLAN, (uint8_t)MSG_LEFT_CLAN, (uint8_t)MSG_CLAN_ACEP,
    (uint8_t)MSG_CLAN_BAN, (uint8_t)MSG_CLAN_KICK, (uint8_t)MSG_CLAN_RECH, (uint8_t)MSG_REV_CLAN,
    (uint8_t)MSG_CHAT, (uint8_t)MSG_TAKE
    }) {
        handlers[type] = [this](const GameMsg& msg) { return serialize_text(msg); };
    };
    for (uint8_t type : {
        (uint8_t)MSG_CHEAT_KILL, (uint8_t)MSG_CHEAT_INF_HP, (uint8_t)MSG_CHEAT_INF_MANA
    }) {
        handlers[type] = [this](const GameMsg& msg) { return serialize_text(msg); };
    };
    handlers[MSG_PRIVATE] = [this](const GameMsg& msg) { return serialize_private(msg); };
    handlers[MSG_NPCS_SNAPSHOT] = [this](const GameMsg& msg) { return serialize_npcs_snapshot(msg); };
    handlers[MSG_ITEMS_SNAPSHOT] = [this](const GameMsg& msg) { return serialize_items_snapshot(msg); };
    handlers[MSG_GOLD] = [this](const GameMsg& msg) { return serialize_gold(msg); };
    handlers[MSG_HP] = [this](const GameMsg& msg) { return serialize_hp(msg); };
    handlers[MSG_XP] = [this](const GameMsg& msg) { return serialize_xp(msg); };
    handlers[MSG_MANA] = [this](const GameMsg& msg) { return serialize_mana(msg); };
    handlers[MSG_PLAYERS_SNAPSHOT] = [this](const GameMsg& msg) { return serialize_players_snapshot(msg); };
    handlers[MSG_ZONE_CHANGE] = [this](const GameMsg& msg) {return serialize_zone(msg); };
    handlers[MSG_ATTACK] = [this](const GameMsg& msg) { return serialize_attack(msg); };
    handlers[MSG_UPDATE_EQUIP] = [this](const GameMsg& msg) {return serialize_update_equip(msg); };
}

// Appendea un uint16_t en big-endian al buffer (definido mas abajo).
static void append_uint16_be(std::vector<uint8_t>& buf, uint16_t value);

void ServerSerializer::write_header(std::vector<uint8_t>& buf, uint8_t type, uint16_t payload_len) {
    buf.push_back(type);
    uint16_t largo_be = htons(payload_len);
    uint8_t largo_bytes[sizeof(uint16_t)];
    std::memcpy(largo_bytes, &largo_be, sizeof(uint16_t));
    buf.insert(buf.end(), largo_bytes, largo_bytes + sizeof(uint16_t));
}

std::vector<uint8_t> ServerSerializer::serialize_inventory(const GameMsg& msg) {
    const std::vector<ItemInfo>& items = msg.get_items();
    uint16_t payload_len = 0;
    for (const auto& item : items) {
        payload_len += static_cast<uint16_t>(item.get_id().size()) + 1
                     + static_cast<uint16_t>(item.get_name().size()) + 1
                     + 1;  // type
    }

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_INVENTORY, payload_len);

    for (const auto& item : items) {
        const std::string& id = item.get_id();
        buf.push_back(static_cast<uint8_t>(id.size()));
        buf.insert(buf.end(), id.begin(), id.end());
        const std::string& name = item.get_name();
        buf.push_back(static_cast<uint8_t>(name.size()));
        buf.insert(buf.end(), name.begin(), name.end());
        buf.push_back(item.get_type());
    }

    return buf;
}


// Formato MSG_MOVE (server -> client):
//   [direction   : LEN_DIRECTION byte]
//   [name_size   : LEN_PLAYER_NAME_SIZE byte][name bytes]
//   [x           : LEN_COORD bytes BE]
//   [y           : LEN_COORD bytes BE]
// El nombre + coords permiten al cliente distinguir su propio jugador de los
// demas y ubicarlos en la posicion correcta.
std::vector<uint8_t> ServerSerializer::serialize_move(const GameMsg& msg) {
    const std::string& name = msg.get_player_name();
    uint16_t payload_len = LEN_DIRECTION
                         + LEN_PLAYER_NAME_SIZE + static_cast<uint16_t>(name.size())
                         + 1
                         + 2 * LEN_COORD;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_MOVE, payload_len);
    buf.push_back(static_cast<uint8_t>(msg.get_direction()));
    buf.push_back(static_cast<uint8_t>(name.size()));
    buf.insert(buf.end(), name.begin(), name.end());
    buf.push_back(static_cast<uint8_t>(RACE_MAP.at(msg.get_race())));
    append_uint16_be(buf, static_cast<uint16_t>(msg.get_coord_x()));
    append_uint16_be(buf, static_cast<uint16_t>(msg.get_coord_y()));
    return buf;
}

// Formato MSG_ZONE_CHANGE: [zona:1B][x:2B BE][y:2B BE]
std::vector<uint8_t> ServerSerializer::serialize_zone(const GameMsg& msg) {
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + LEN_ZONE_CHANGE_PAYLOAD);
    write_header(buf, MSG_ZONE_CHANGE, LEN_ZONE_CHANGE_PAYLOAD);
    buf.push_back(static_cast<uint8_t>(msg.get_zone()));
    append_uint16_be(buf, static_cast<uint16_t>(msg.get_coord_x()));
    append_uint16_be(buf, static_cast<uint16_t>(msg.get_coord_y()));
    return buf;
}

// MSG_ATTACK (server->cliente): [x:2B BE][y:2B BE][damage:2B BE]
std::vector<uint8_t> ServerSerializer::serialize_attack(const GameMsg& msg) {
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + LEN_ATTACK_PAYLOAD);
    write_header(buf, MSG_ATTACK, LEN_ATTACK_PAYLOAD);
    append_uint16_be(buf, static_cast<uint16_t>(msg.get_coord_x()));
    append_uint16_be(buf, static_cast<uint16_t>(msg.get_coord_y()));
    append_uint16_be(buf, static_cast<uint16_t>(msg.get_damage()));
    return buf;
}

std::vector<uint8_t> ServerSerializer::serialize_map(const GameMsg& msg) {
    const auto& map = msg.get_map();
    uint16_t payload_len = 0;
    for (const auto& row : map) {
        payload_len += static_cast<uint16_t>(row.size());
    }

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_SEND_MAP, payload_len);

    for (const auto& row : map) {
        for (elements cell : row) {
            auto it = ELEMENT_TYPE_MAP.find(cell);
            uint8_t encoded = (it != ELEMENT_TYPE_MAP.end()) ? static_cast<uint8_t>(it->second)
                                                              : static_cast<uint8_t>(ELEMENT_EMPTY);
            buf.push_back(encoded);
        }
    }

    return buf;
}

std::vector<uint8_t> ServerSerializer::serialize_cmd(const GameMsg& msg) {
    auto it = handlers.find(msg.get_type());
    if (it != handlers.end()) {
        return it->second(msg);
    }
    return {};
}

std::vector<uint8_t> ServerSerializer::serialize_text(const GameMsg& msg) {
    const std::string& content = msg.get_chat_content();
    uint16_t payload_len = LEN_NAME_SIZE_FIELD + static_cast<uint16_t>(content.size());

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, static_cast<uint8_t>(msg.get_type()), payload_len);
    buf.push_back(static_cast<uint8_t>(content.size()));
    buf.insert(buf.end(), content.begin(), content.end());
    return buf;
}

std::vector<uint8_t> ServerSerializer::serialize_name(const GameMsg& msg) {
    const std::string& name = msg.get_player_name();
    uint16_t payload_len = LEN_NAME_SIZE_FIELD + static_cast<uint16_t>(name.size());

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, static_cast<uint8_t>(msg.get_type()), payload_len);
    buf.push_back(static_cast<uint8_t>(name.size()));
    buf.insert(buf.end(), name.begin(), name.end());
    return buf;
}

// Formato MSG_UPDATE_EQUIP:
//   [name_size:1B][name][equipped:1B][n_ids:1B] luego n veces: [id_size:1B][id]
// 'equipped' indica si el jugador tiene un arma equipada (para el sprite);
// 'n_ids'/ids son TODOS los items equipados (arma + defensas) para resaltar el inventario.
std::vector<uint8_t> ServerSerializer::serialize_update_equip(const GameMsg& msg) {
    const std::string& name = msg.get_player_name();
    const std::vector<std::string>& ids = msg.get_equipped_ids();

    uint16_t payload_len = LEN_NAME_SIZE_FIELD + static_cast<uint16_t>(name.size()) + 1 + 1;
    for (const std::string& id : ids) {
        payload_len += LEN_NAME_SIZE_FIELD + static_cast<uint16_t>(id.size());
    }

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, static_cast<uint8_t>(msg.get_type()), payload_len);
    buf.push_back(static_cast<uint8_t>(name.size()));
    buf.insert(buf.end(), name.begin(), name.end());
    buf.push_back(msg.get_equipped() ? 1 : 0);
    buf.push_back(static_cast<uint8_t>(ids.size()));
    for (const std::string& id : ids) {
        buf.push_back(static_cast<uint8_t>(id.size()));
        buf.insert(buf.end(), id.begin(), id.end());
    }
    return buf;
}

// Helper: appendea un uint16_t en big-endian al buffer.
static void append_uint16_be(std::vector<uint8_t>& buf, uint16_t value) {
    uint16_t be = htons(value);
    uint8_t bytes[LEN_COORD];
    std::memcpy(bytes, &be, LEN_COORD);
    buf.insert(buf.end(), bytes, bytes + LEN_COORD);
}

// Formato MSG_NPCS_SNAPSHOT:
//   [count          : LEN_NPC_COUNT bytes BE]
//   foreach npc:
//     [type_size : LEN_NPC_TYPE_SIZE byte][type bytes]
//     [name_size : LEN_NPC_NAME_SIZE byte][name bytes]
//     [x         : LEN_COORD bytes BE]
//     [y         : LEN_COORD bytes BE]
std::vector<uint8_t> ServerSerializer::serialize_npcs_snapshot(const GameMsg& msg) {
    const auto& npcs = msg.get_npcs();

    uint16_t payload_len = LEN_NPC_COUNT;
    for (const auto& n : npcs) {
        payload_len += LEN_NPC_TYPE_SIZE + static_cast<uint16_t>(n.type.size());
        payload_len += LEN_NPC_NAME_SIZE + static_cast<uint16_t>(n.name.size());
        payload_len += 2 * LEN_COORD;
    }

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_NPCS_SNAPSHOT, payload_len);

    append_uint16_be(buf, static_cast<uint16_t>(npcs.size()));

    for (const auto& n : npcs) {
        buf.push_back(static_cast<uint8_t>(n.type.size()));
        buf.insert(buf.end(), n.type.begin(), n.type.end());
        buf.push_back(static_cast<uint8_t>(n.name.size()));
        buf.insert(buf.end(), n.name.begin(), n.name.end());
        append_uint16_be(buf, static_cast<uint16_t>(n.x));
        append_uint16_be(buf, static_cast<uint16_t>(n.y));
    }

    return buf;
}

// Formato MSG_ITEMS_SNAPSHOT:
//   [count          : LEN_ITEM_COUNT bytes BE]
//   foreach item:
//     [type_size : LEN_ITEM_TYPE_SIZE byte][type bytes]
//     [x         : LEN_COORD bytes BE]
//     [y         : LEN_COORD bytes BE]
std::vector<uint8_t> ServerSerializer::serialize_items_snapshot(const GameMsg& msg) {
    const auto& items = msg.get_items_on_floor();

    uint16_t payload_len = LEN_ITEM_COUNT;
    for (const auto& i : items) {
        payload_len += LEN_ITEM_TYPE_SIZE + static_cast<uint16_t>(i.type.size());
        payload_len += 2 * LEN_COORD;
    }

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_ITEMS_SNAPSHOT, payload_len);

    append_uint16_be(buf, static_cast<uint16_t>(items.size()));

    for (const auto& i : items) {
        buf.push_back(static_cast<uint8_t>(i.type.size()));
        buf.insert(buf.end(), i.type.begin(), i.type.end());
        append_uint16_be(buf, static_cast<uint16_t>(i.x));
        append_uint16_be(buf, static_cast<uint16_t>(i.y));
    }

    return buf;
}

std::vector<uint8_t> ServerSerializer::serialize_private(const GameMsg& msg) {
    const std::string& sender = msg.get_player_name();
    const std::string& content = msg.get_chat_content();
    uint16_t payload_len = 1 + sender.size() + 1 + content.size();
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_PRIVATE, payload_len);
    buf.push_back(static_cast<uint8_t>(sender.size()));
    buf.insert(buf.end(), sender.begin(), sender.end());
    buf.push_back(static_cast<uint8_t>(content.size()));
    buf.insert(buf.end(), content.begin(), content.end());
    return buf;
}

std::vector<uint8_t> ServerSerializer::serialize_value(const GameMsg& msg, uint32_t value) {
    std::vector<uint8_t> buf;
    uint16_t payload_len = sizeof(uint32_t);

    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, static_cast<uint8_t>(msg.get_type()), payload_len);
    uint32_t value_be = htonl(value);
    uint8_t value_bytes[sizeof(uint32_t)];
    std::memcpy(value_bytes, &value_be, sizeof(uint32_t));
    buf.insert(buf.end(), value_bytes, value_bytes + sizeof(uint32_t));

    return buf;
}

std::vector<uint8_t> ServerSerializer::serialize_gold(const GameMsg& msg) {
    uint32_t gold = msg.get_gold();

    return serialize_value(msg, gold);
}

std::vector<uint8_t> ServerSerializer::serialize_hp(const GameMsg& msg) {
    uint32_t hp = msg.get_hp();

    return serialize_value(msg, hp);
}

std::vector<uint8_t> ServerSerializer::serialize_xp(const GameMsg& msg) {
    uint32_t xp = msg.get_xp();

    return serialize_value(msg, xp);
}

std::vector<uint8_t> ServerSerializer::serialize_mana(const GameMsg& msg) {
    uint32_t mana = msg.get_mana();

    return serialize_value(msg, mana);
}

// Formato MSG_REGISTER payload:
//   [map_rows        : 2 bytes BE]
//   [map_cols        : 2 bytes BE]
//   foreach row, col:
//     [cell           : 1 byte]
//   [item_count      : 1 byte]
//   foreach item:
//     [id_size        : 1 byte][id bytes]
//     [name_size      : 1 byte][name bytes]
//   [gold            : 4 bytes BE]
//   [hp              : 4 bytes BE]
//   [xp              : 4 bytes BE]
//   [mana            : 4 bytes BE]
//   [spawn_x         : 2 bytes BE]   posicion inicial del jugador en el mapa
//   [spawn_y         : 2 bytes BE]
//   [player_count    : 2 bytes BE]   jugadores ya en el mapa al momento del registro
//   foreach player:
//     [name_size     : 1 byte][name bytes]
//     [x             : 2 bytes BE]
//     [y             : 2 bytes BE]
std::vector<uint8_t> ServerSerializer::serialize_register(const GameMsg& msg) {
    const auto& map = msg.get_map();
    const auto& items = msg.get_items();

    uint16_t rows = static_cast<uint16_t>(map.size());
    uint16_t cols = (rows > 0) ? static_cast<uint16_t>(map[0].size()) : 0;

    uint16_t payload_len = 0;
    payload_len += 2 * LEN_COORD;           // rows + cols
    payload_len += rows * cols;             // celdas del mapa
    payload_len += 1;                       // item_count
    for (const auto& item : items) {
        payload_len += 1 + static_cast<uint16_t>(item.get_id().size());
        payload_len += 1 + static_cast<uint16_t>(item.get_name().size());
        payload_len += 1;  // type
    }
    payload_len += 4 * sizeof(uint32_t);    // gold + hp + xp + mana
    payload_len += 2 * LEN_COORD;           // spawn_x + spawn_y
    const auto& players = msg.get_players();
    payload_len += LEN_PLAYER_COUNT;
    for (const auto& pl : players) {
        payload_len += LEN_PLAYER_NAME_SIZE + static_cast<uint16_t>(pl.name.size());
        payload_len += 1;                   // race
        payload_len += 2 * LEN_COORD;       // x + y
    }

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_REGISTER, payload_len);

    append_uint16_be(buf, rows);
    append_uint16_be(buf, cols);
    for (const auto& row : map) {
        for (elements cell : row) {
            auto it = ELEMENT_TYPE_MAP.find(cell);
            uint8_t encoded = (it != ELEMENT_TYPE_MAP.end()) ? static_cast<uint8_t>(it->second)
                                                              : static_cast<uint8_t>(ELEMENT_EMPTY);
            buf.push_back(encoded);
        }
    }

    buf.push_back(static_cast<uint8_t>(items.size()));
    for (const auto& item : items) {
        const std::string& id = item.get_id();
        buf.push_back(static_cast<uint8_t>(id.size()));
        buf.insert(buf.end(), id.begin(), id.end());
        const std::string& name = item.get_name();
        buf.push_back(static_cast<uint8_t>(name.size()));
        buf.insert(buf.end(), name.begin(), name.end());
        buf.push_back(item.get_type());
    }

    auto append_u32 = [&](uint32_t value) {
        uint32_t be = htonl(value);
        uint8_t bytes[sizeof(uint32_t)];
        std::memcpy(bytes, &be, sizeof(uint32_t));
        buf.insert(buf.end(), bytes, bytes + sizeof(uint32_t));
    };
    append_u32(msg.get_gold());
    append_u32(msg.get_hp());
    append_u32(msg.get_xp());
    append_u32(msg.get_mana());
    append_uint16_be(buf, static_cast<uint16_t>(msg.get_coord_x()));
    append_uint16_be(buf, static_cast<uint16_t>(msg.get_coord_y()));

    append_uint16_be(buf, static_cast<uint16_t>(players.size()));
    for (const auto& pl : players) {
        buf.push_back(static_cast<uint8_t>(pl.name.size()));
        buf.insert(buf.end(), pl.name.begin(), pl.name.end());
        uint8_t race_code = RACE_HUMAN;
        if (pl.race == "elf") {
            race_code = RACE_ELF;
        } else if (pl.race == "dwarf") {
            race_code = RACE_DWARF;
        } else if (pl.race == "gnome") {
            race_code = RACE_GNOME;
        }
        buf.push_back(race_code);
        append_uint16_be(buf, static_cast<uint16_t>(pl.x));
        append_uint16_be(buf, static_cast<uint16_t>(pl.y));
    }

    return buf;
}

// Formato MSG_PLAYERS_SNAPSHOT:
//   [count     : LEN_PLAYER_COUNT bytes BE]   cantidad de otros jugadores en el mapa
//   foreach player:
//     [name_size : LEN_PLAYER_NAME_SIZE byte]  largo del nombre
//     [name bytes]                             nombre del jugador
//     [x         : LEN_COORD bytes BE]         columna en el mapa
//     [y         : LEN_COORD bytes BE]         fila en el mapa
std::vector<uint8_t> ServerSerializer::serialize_players_snapshot(const GameMsg& msg) {
    const auto& players = msg.get_players();

    // Calculamos el payload total antes de reservar para evitar reallocations.
    uint16_t payload_len = LEN_PLAYER_COUNT;
    for (const auto& p : players) {
        payload_len += LEN_PLAYER_NAME_SIZE + static_cast<uint16_t>(p.name.size());
        payload_len += 2 * LEN_COORD;  // x + y
    }

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_PLAYERS_SNAPSHOT, payload_len);

    // Cantidad de jugadores en el snapshot (big-endian).
    append_uint16_be(buf, static_cast<uint16_t>(players.size()));

    for (const auto& p : players) {
        buf.push_back(static_cast<uint8_t>(p.name.size()));
        buf.insert(buf.end(), p.name.begin(), p.name.end());
        append_uint16_be(buf, static_cast<uint16_t>(p.x));
        append_uint16_be(buf, static_cast<uint16_t>(p.y));
    }

    return buf;
}
