#include "parser.h"
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

ClientCmd Parser::parse_chat(const std::string& input) {
    std::istringstream ss(input);
    std::string command;
    ss >> command;

    ClientCmd cmd;

    if (command == "/meditar") {
        cmd.set_message_type(MSG_MEDITATE);
    } else if (command == "/fundar-clan") {
        std::string clan_name;
        if (!(ss >> clan_name)) {
            throw std::invalid_argument("Uso: /fundar-clan <nombre>");
        }
        cmd.set_message_type(MSG_FOUND_CLAN);
        cmd.set_target_name(clan_name);
    } else if (command == "/resucitar") {
        cmd.set_message_type(MSG_RESURRECT);
    } else if (command == "/curar") {
        cmd.set_message_type(MSG_CURE);
    } else if (command == "/depositar") {
        std::string item;
        if (!(ss >> item)) {
            throw std::invalid_argument("Uso: /depositar <objeto>");
        }
        cmd.set_message_type(MSG_DEPOSIT);
        cmd.set_item_id(item);
    } else if (command == "/retirar") {
        std::string item;
        if (!(ss >> item)) {
            throw std::invalid_argument("Uso: /retirar <objeto>");
        }
        cmd.set_message_type(MSG_RETIRE);
        cmd.set_item_id(item);
    } else if (command == "/listar") {
        cmd.set_message_type(MSG_LIST);
    } else if (command == "/comprar") {
        std::string item;
        if (!(ss >> item)) {
            throw std::invalid_argument("Uso: /comprar <objeto>");
        }
        cmd.set_message_type(MSG_BUY);
        cmd.set_item_id(item);
    } else if (command == "/vender") {
        std::string item;
        if (!(ss >> item)) {
            throw std::invalid_argument("Uso: /vender <objeto>");
        }
        cmd.set_message_type(MSG_SELL);
        cmd.set_item_id(item);
    } else if (command == "/tomar") {
        cmd.set_message_type(MSG_TAKE);
    } else if (command == "/tirar") {
        cmd.set_message_type(MSG_THROW);
    } else if (command == "/unirse") {
        std::string clan_name;
        if (!(ss >> clan_name)) {
            throw std::invalid_argument("Uso: /unirse <nombre>");
        }
        cmd.set_message_type(MSG_JOIN_CLAN);
        cmd.set_target_name(clan_name);
    } else if (command == "/revisar-clan") {
        cmd.set_message_type(MSG_REV_CLAN);
    } else if (command == "/clan-aceptar") {
        std::string player;
        if (!(ss >> player)) {
            throw std::invalid_argument("Uso: /clan-aceptar <nombre>");
        }
        cmd.set_message_type(MSG_CLAN_ACEP);
        cmd.set_target_name(player);
    } else if (command == "/clan-rechazar") {
        std::string player;
        if (!(ss >> player)) {
            throw std::invalid_argument("Uso: /clan-rechazar <nombre>");
        }
        cmd.set_message_type(MSG_CLAN_RECH);
        cmd.set_target_name(player);
    } else if (command == "/clan-ban") {
        std::string player;
        if (!(ss >> player)) {
            throw std::invalid_argument("Uso: /clan-ban <nombre>");
        }
        cmd.set_message_type(MSG_CLAN_BAN);
        cmd.set_target_name(player);
    } else if (command == "/clan-kick") {
        std::string player;
        if (!(ss >> player)) {
            throw std::invalid_argument("Uso: /clan-kick <nombre>");
        }
        cmd.set_message_type(MSG_CLAN_KICK);
        cmd.set_target_name(player);
    } else if (command == "/dejar-clan") {
        cmd.set_message_type(MSG_LEFT_CLAN);
    } else if (command == "/dep-oro") {
        uint32_t amount;
        if (!(ss >> amount)) {
            throw std::invalid_argument("Uso: /dep-oro <cantidad>");
        }
        cmd.set_message_type(MSG_DEP_GOLD);
        cmd.set_gold(amount);
    } else if (command == "/ret-oro") {
        uint32_t amount;
        if (!(ss >> amount)) {
            throw std::invalid_argument("Uso: /ret-oro <cantidad>");
        }
        cmd.set_message_type(MSG_RET_GOLD);
        cmd.set_gold(amount);
    } else if (command == "/cheat-morir") {
        cmd.set_message_type(MSG_CHEAT_KILL);
    } else if (command == "/cheat-vida") {
        cmd.set_message_type(MSG_CHEAT_INF_HP);
    } else if (command == "/cheat-mana") {
        cmd.set_message_type(MSG_CHEAT_INF_MANA);
    } else {
        throw std::invalid_argument("Comando desconocido: " + command);
    }

    return cmd;
}
