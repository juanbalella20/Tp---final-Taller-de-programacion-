#include "serializer.h"
#include "../common/protocol_constants.h"
#include <arpa/inet.h>
#include <cstring>

Serializer::Serializer() {
    handlers[MSG_REGISTER] = [this](const ClientCmd& cmd) { return serialize_register(cmd); };
    handlers[MSG_MOVE]     = [this](const ClientCmd& cmd) { return serialize_move(cmd); };
    handlers[MSG_ATTACK]   = [this](const ClientCmd& cmd) { return serialize_coords(MSG_ATTACK, cmd); };
    handlers[MSG_SELECT]   = [this](const ClientCmd& cmd) { return serialize_coords(MSG_SELECT, cmd); };
    handlers[MSG_BUY]      = [this](const ClientCmd& cmd) { return serialize_item_id(MSG_BUY, cmd); };
    handlers[MSG_SELL]     = [this](const ClientCmd& cmd) { return serialize_item_id(MSG_SELL, cmd); };
    handlers[MSG_EQUIP]    = [this](const ClientCmd& cmd) { return serialize_item_id(MSG_EQUIP, cmd); };
    handlers[MSG_THROW]    = [this](const ClientCmd& cmd) { return serialize_item_id(MSG_THROW, cmd); };
    handlers[MSG_DEPOSIT]  = [this](const ClientCmd& cmd) { return serialize_item_id(MSG_DEPOSIT, cmd); };
    handlers[MSG_RETIRE]   = [this](const ClientCmd& cmd) { return serialize_item_id(MSG_RETIRE, cmd); };
    handlers[MSG_MEDITATE] = [this](const ClientCmd& cmd) { return serialize_no_payload(MSG_MEDITATE); };
    handlers[MSG_RESURRECT]= [this](const ClientCmd& cmd) { return serialize_no_payload(MSG_RESURRECT); };
    handlers[MSG_CURE]     = [this](const ClientCmd& cmd) { return serialize_no_payload(MSG_CURE); };
    handlers[MSG_LIST]     = [this](const ClientCmd& cmd) { return serialize_no_payload(MSG_LIST); };
    handlers[MSG_REV_CLAN] = [this](const ClientCmd& cmd) { return serialize_no_payload(MSG_REV_CLAN); };
    handlers[MSG_LEFT_CLAN] = [this](const ClientCmd& cmd) { return serialize_no_payload(MSG_LEFT_CLAN); };
    handlers[MSG_FOUND_CLAN] = [this](const ClientCmd& cmd) { return serialize_name(MSG_FOUND_CLAN, cmd); };
    handlers[MSG_JOIN_CLAN]  = [this](const ClientCmd& cmd) { return serialize_name(MSG_JOIN_CLAN,  cmd); };
    handlers[MSG_CLAN_ACEP]  = [this](const ClientCmd& cmd) { return serialize_name(MSG_CLAN_ACEP,  cmd); };
    handlers[MSG_CLAN_RECH] = [this](const ClientCmd& cmd) { return serialize_name(MSG_CLAN_RECH, cmd); };
    handlers[MSG_CLAN_BAN]  = [this](const ClientCmd& cmd) { return serialize_name(MSG_CLAN_BAN,  cmd); };
    handlers[MSG_CLAN_KICK] = [this](const ClientCmd& cmd) { return serialize_name(MSG_CLAN_KICK, cmd); };
    handlers[MSG_DEP_GOLD]  = [this](const ClientCmd& cmd) { return serialize_gold(MSG_DEP_GOLD, cmd); };
    handlers[MSG_RET_GOLD] = [this](const ClientCmd& cmd) { return serialize_gold(MSG_RET_GOLD, cmd); };
    handlers[MSG_CHEAT_KILL] = [this](const ClientCmd& cmd) { return serialize_no_payload(MSG_CHEAT_KILL); };
    handlers[MSG_CHEAT_INF_HP] = [this](const ClientCmd& cmd) { return serialize_no_payload(MSG_CHEAT_INF_HP); };
    handlers[MSG_CHEAT_INF_MANA] = [this](const ClientCmd& cmd) { return serialize_no_payload(MSG_CHEAT_INF_MANA); };
    handlers[MSG_PRIVATE] = [this](const ClientCmd& cmd) { return serialize_private(cmd); };
}

void Serializer::write_header(std::vector<uint8_t>& buf, uint8_t type, uint16_t payload_len) {
    buf.push_back(type);
    uint16_t largo_be = htons(payload_len);
    uint8_t largo_bytes[sizeof(uint16_t)];
    std::memcpy(largo_bytes, &largo_be, sizeof(uint16_t));//void* memcpy(void* destino, const void* origen, size_t cantidad_de_bytes);
    // memcpy se usa para copiar los bytes de largo_be (en formato big-endian) al arreglo largo_bytes,
    //que luego se inserta en el buffer de salida. 
    //Esto asegura que el header del mensaje tenga el formato correcto esperado por el protocolo de comunicación.
    buf.insert(buf.end(), largo_bytes, largo_bytes + sizeof(uint16_t));
    
}


std::vector<uint8_t> Serializer::serialize_register(const ClientCmd& cmd) {
    const std::string& name = cmd.get_player_name();
    uint8_t name_len = static_cast<uint8_t>(name.size());
    uint16_t payload_len = LEN_NAME_SIZE_FIELD + name_len + LEN_RACE + LEN_CLASS;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_REGISTER, payload_len);
    buf.push_back(name_len);
    buf.insert(buf.end(), name.begin(), name.end());
    buf.push_back(static_cast<uint8_t>(RACE_MAP.at(cmd.get_race())));
    buf.push_back(static_cast<uint8_t>(CLASS_MAP.at(cmd.get_class())));
    return buf;
}

std::vector<uint8_t> Serializer::serialize_move(const ClientCmd& cmd) {
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + 1);
    write_header(buf, MSG_MOVE, 1);
    buf.push_back(static_cast<uint8_t>(cmd.get_direction()));
    return buf;
}

std::vector<uint8_t> Serializer::serialize_coords(uint8_t type, const ClientCmd& cmd) {
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + LEN_COORD + LEN_COORD);
    write_header(buf, type, LEN_COORD + LEN_COORD);
    uint16_t x_be = htons(static_cast<uint16_t>(cmd.get_coord_x()));
    uint16_t y_be = htons(static_cast<uint16_t>(cmd.get_coord_y()));
    uint8_t tmp[2];
    std::memcpy(tmp, &x_be, LEN_COORD);
    buf.insert(buf.end(), tmp, tmp + LEN_COORD);
    std::memcpy(tmp, &y_be, LEN_COORD);
    buf.insert(buf.end(), tmp, tmp + LEN_COORD);
    return buf;
}
/*
std::vector<uint8_t> Serializer::serialize_entity_and_name(uint8_t type, const ClientCmd& cmd) {
    const std::string& target = cmd.get_target_name();
    uint8_t target_len = static_cast<uint8_t>(target.size());
    uint16_t payload_len = LEN_ENTITY + LEN_NAME_SIZE_FIELD + target_len;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, type, payload_len);
    buf.push_back(static_cast<uint8_t>(cmd.get_target_type()));
    buf.push_back(target_len);
    buf.insert(buf.end(), target.begin(), target.end());
    return buf;
}
*/
std::vector<uint8_t> Serializer::serialize_name(uint8_t type, const ClientCmd& cmd) {
    const std::string& target = cmd.get_target_name();
    uint8_t target_len = static_cast<uint8_t>(target.size());
    uint16_t payload_len = LEN_NAME_SIZE_FIELD + target_len;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, type, payload_len);
    buf.push_back(target_len);
    buf.insert(buf.end(), target.begin(), target.end());
    return buf;
}

std::vector<uint8_t> Serializer::serialize_item_id(uint8_t type, const ClientCmd& cmd) {
    const std::string& item_id = cmd.get_item_id();
    uint8_t item_id_len = static_cast<uint8_t>(item_id.size());
    uint16_t payload_len = LEN_NAME_SIZE_FIELD + item_id_len;

    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, type, payload_len);
    buf.push_back(item_id_len);
    buf.insert(buf.end(), item_id.begin(), item_id.end());
    return buf;
}

std::vector<uint8_t> Serializer::serialize_no_payload(uint8_t type) {
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER);
    write_header(buf, type, 0);
    return buf;
}

std::vector<uint8_t> Serializer::serialize_gold(uint8_t type, const ClientCmd& cmd) {
    std::vector<uint8_t> buf;
    buf.reserve(LEN_HEADER + sizeof(uint32_t));
    write_header(buf, type, sizeof(uint32_t));
    uint32_t amount_be = htonl(cmd.get_gold());
    uint8_t amount_bytes[sizeof(uint32_t)];
    std::memcpy(amount_bytes, &amount_be, sizeof(uint32_t));
    buf.insert(buf.end(), amount_bytes, amount_bytes + sizeof(uint32_t));
    return buf;
}

std::vector<uint8_t> Serializer::serialize_cmd(const ClientCmd& cmd) {
    auto it = handlers.find(cmd.get_message_type());
    if (it != handlers.end()) {
        return it->second(cmd);
    }
    return {};
}

std::vector<uint8_t> Serializer::serialize_private(const ClientCmd& cmd) {
    std::vector<uint8_t> buf;
    const std::string& target = cmd.get_target_name();
    const std::string& content = cmd.get_chat_content();
    uint8_t target_len = static_cast<uint8_t>(target.size());
    uint8_t content_len = static_cast<uint8_t>(content.size());
    uint16_t payload_len = 1 + target_len + 1 + content_len;
    buf.reserve(LEN_HEADER + payload_len);
    write_header(buf, MSG_PRIVATE, payload_len);
    buf.push_back(target_len);
    buf.insert(buf.end(), target.begin(), target.end());
    buf.push_back(content_len);
    buf.insert(buf.end(), content.begin(), content.end());
    return buf;
}
