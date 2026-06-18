#include "parser.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

// Formato esperado: "register <nombre> <raza> <clase>"
ClientCmd Parser::parse(const std::string& input) {
    std::istringstream ss(input);
    std::string command;
    ss >> command;

    ClientCmd cmd;

    if (command == "register") {
        std::string name, race, klass;
        if (!(ss >> name >> race >> klass))
            throw std::invalid_argument("Uso: register <nombre> <raza> <clase>");
        cmd.set_message_type(MSG_REGISTER);
        cmd.set_player_name(name);
        cmd.set_race(race);
        cmd.set_class(klass);
    } else if (command == "w") {
        cmd.set_message_type(MSG_MOVE);
        cmd.set_direction(DIR_NORTH);
    } else if (command == "s") {
        cmd.set_message_type(MSG_MOVE);
        cmd.set_direction(DIR_SOUTH);
    } else if (command == "a") {
        cmd.set_message_type(MSG_MOVE);
        cmd.set_direction(DIR_WEST);
    } else if (command == "d") {
        cmd.set_message_type(MSG_MOVE);
        cmd.set_direction(DIR_EAST);
    } else if (command == "attack") {
        std::string entity_type, target_name;
        if (!(ss >> entity_type >> target_name))
            throw std::invalid_argument("Uso: attack <player|npc> <nombre>");
        cmd.set_message_type(MSG_ATTACK);
        cmd.set_target_type(entity_type == "npc" ? ENTITY_NPC : ENTITY_PLAYER);
        cmd.set_target_name(target_name);
    } else {
        throw std::invalid_argument("Comando desconocido: " + command);
    }

    return cmd;
}

ClientCmd Parser::parse_private(const std::string& command, std::istringstream& ss) {
    std::string target = command.substr(1);
    if (target.empty()) {
        throw std::invalid_argument("Uso: @<nick> <mensaje>");
    }
    std::string content;
    std::getline(ss, content);
    if (!content.empty() && content.front() == ' ') {
        content = content.substr(1);
    }
    ClientCmd cmd;
    cmd.set_message_type(MSG_PRIVATE);
    cmd.set_target_name(target);
    cmd.set_chat_content(content);
    return cmd;
}

ClientCmd Parser::parse_no_payload(MessageType type) {
    ClientCmd cmd;
    cmd.set_message_type(type);
    return cmd;
}

ClientCmd Parser::parse_tp_zone(MessageType type, std::string zone) {
    ClientCmd cmd;
    cmd.set_message_type(type);
    if (zone == "desert") {
        cmd.set_zone(ZONE_DESERT);
    } else if (zone == "city") {
        cmd.set_zone(ZONE_CITY);
    } else if (zone == "forest") {
        cmd.set_zone(ZONE_FOREST);
    } else if (zone == "town") {
        cmd.set_zone(ZONE_TOWN);
    } else {
        throw std::invalid_argument(
            "Zona invalida: '" + zone + "'. Use: desert | city | forest | town");
    }
    return cmd;
}

ClientCmd Parser::parse_item_cmd(MessageType type, std::istringstream& ss, const std::string& correct_use) {
    std::string item;
    if (!std::getline(ss >> std::ws, item) || item.empty()) {
        throw std::invalid_argument(correct_use);
    }
    std::replace(item.begin(), item.end(), ' ', '_');
    ClientCmd cmd;
    cmd.set_message_type(type);
    cmd.set_item_id(item);
    return cmd;
}

ClientCmd Parser::parse_gold_cmd(MessageType type, std::istringstream& ss, const std::string& correct_use) {
    int32_t amount;
    if (!(ss >> amount)) {
        throw std::invalid_argument(correct_use);
    }
    ClientCmd cmd;
    cmd.set_message_type(type);
    cmd.set_gold(amount);
    return cmd;
}

ClientCmd Parser::parse_name_cmd(MessageType type, std::istringstream& ss, const std::string& correct_use) {
    std::string name;
    if (!std::getline(ss >> std::ws, name) || name.empty()) {
        throw std::invalid_argument(correct_use);
    }

    size_t end = name.find_last_not_of(" \t\r\n");
    if (end != std::string::npos) {
        name = name.substr(0, end + 1);
    }

    ClientCmd cmd;
    cmd.set_message_type(type);
    cmd.set_target_name(name);
    return cmd;
}

bool Parser::health_command(const std::string& command, ClientCmd& cmd) {
    if (command == "/meditar") {
        cmd = parse_no_payload(MSG_MEDITATE);
        return true;
    }
    if (command == "/resucitar") {
        cmd = parse_no_payload(MSG_RESURRECT);
        return true;
    }
    if (command == "/curar") {
        cmd = parse_no_payload(MSG_CURE);
        return true;
    }
    return false;
}

bool Parser::cheat_command(const std::string& command, ClientCmd& cmd, std::istringstream& ss) {
    if (command == "/cheat-morir") {
        cmd = parse_no_payload(MSG_CHEAT_KILL);
        return true;
    }
    if (command == "/cheat-vida") {
        cmd = parse_no_payload(MSG_CHEAT_INF_HP);
        return true;
    }
    if (command == "/cheat-mana") {
        cmd = parse_no_payload(MSG_CHEAT_INF_MANA);
        return true;
    }
    if (command == "/mana") {
        // Cheat: resta N de maná (para probar la recuperación con /meditar).
        cmd = parse_gold_cmd(MSG_CHEAT_MANA, ss, "Uso: /mana <cantidad>");
        return true;
    }
    if (command == "/cheat-resucitar") {
        cmd = parse_no_payload(MSG_CHEAT_RESPAWN);
        return true;
    }
    return false;
}

bool Parser::item_related_command(const std::string& command, ClientCmd& cmd, std::istringstream& ss) {
    if (command == "/depositar") {
        cmd = parse_item_cmd(MSG_DEPOSIT, ss, "Uso: /depositar <objeto>");
        return true;
    } 
    if (command == "/retirar") {
        cmd = parse_item_cmd(MSG_RETIRE, ss, "Uso: /retirar <objeto>");
        return true;
    } 
    if (command == "/listar") {
        cmd = parse_no_payload(MSG_LIST);
        return true;
    } 
    if (command == "/comprar") {
        cmd = parse_item_cmd(MSG_BUY, ss, "Uso: /comprar <objeto>");
        return true;
    } 
    if (command == "/vender") {
        cmd = parse_item_cmd(MSG_SELL, ss, "Uso: /vender <objeto>");
        return true;
    }
    if (command == "/tomar") {
        cmd = parse_no_payload(MSG_TAKE);
        return true;
    } 
    if (command == "/tirar") {
        cmd = parse_no_payload(MSG_THROW);
        return true;
    }
    return false;
}

bool Parser::clan_related_command(const std::string& command, ClientCmd& cmd, std::istringstream& ss) {
    if (command == "/fundar-clan") {
        cmd = parse_name_cmd(MSG_FOUND_CLAN, ss, "Uso: /fundar-clan <nombre>");
        return true;
    }
    if (command == "/unirse") {
        cmd = parse_name_cmd(MSG_JOIN_CLAN, ss, "Uso: /unirse <nombre>");
        return true;
    }
    if (command == "/revisar-clan") {
        cmd = parse_no_payload(MSG_REV_CLAN);
        return true;
    } 
    if (command == "/clan-aceptar") {
        cmd = parse_name_cmd(MSG_CLAN_ACEP, ss, "Uso: /clan-aceptar <nombre>");
        return true;
    } 
    if (command == "/clan-rechazar") {
        cmd = parse_name_cmd(MSG_CLAN_RECH, ss, "Uso: /clan-rechazar <nombre>");
        return true;
    }
    if (command == "/clan-ban") {
        cmd = parse_name_cmd(MSG_CLAN_BAN, ss, "Uso: /clan-ban <nombre>");
        return true;
    }
    if (command == "/clan-kick") {
        cmd = parse_name_cmd(MSG_CLAN_KICK, ss, "Uso: /clan-kick <nombre>");
        return true;
    } 
    if (command == "/dejar-clan") {
        cmd = parse_no_payload(MSG_LEFT_CLAN);
        return true;
    }
    return false;
}

bool Parser::gold_related_command(const std::string& command, ClientCmd& cmd, std::istringstream& ss) {
    if (command == "/dep-oro") {
        cmd = parse_gold_cmd(MSG_DEP_GOLD, ss, "Uso: /dep-oro <cantidad>");
        return true;
    }
    if (command == "/ret-oro") {
        cmd = parse_gold_cmd(MSG_RET_GOLD, ss, "Uso: /ret-oro <cantidad>");
        return true;
    }
    return false;
}

bool Parser::teleport_command(const std::string& command, ClientCmd& cmd,
                              std::istringstream& ss) {
    // CHEAT: "/tp <zona>". El nombre de zona se lee del stream (parse_chat ya
    // consumio "/tp" como 'command'), igual que el resto de comandos con args.
    if (command == "/tp") {
        std::string zone;
        if (!(ss >> zone)) {
            throw std::invalid_argument("Uso: /tp <desert|city|forest|town>");
        }
        cmd = parse_tp_zone(MSG_TELEPORT, zone);
        return true;
    }
    return false;
}

ClientCmd Parser::parse_chat(const std::string& input) {
    std::istringstream ss(input);
    std::string command;
    ss >> command;

    ClientCmd cmd;

    if (command.front() == '@') {
        return parse_private(command, ss);
    }

    if (health_command(command, cmd)) {
        return cmd;
    }
    if (item_related_command(command, cmd, ss)) {
        return cmd;
    }
    if (clan_related_command(command, cmd, ss)) {
        return cmd;
    }
    if (gold_related_command(command, cmd, ss)) {
        return cmd;
    }
    if (cheat_command(command, cmd, ss)) {
        return cmd;
    }
    if (teleport_command(command, cmd, ss)) {
        return cmd;
    }

    throw std::invalid_argument("Comando desconocido: " + command);
}
