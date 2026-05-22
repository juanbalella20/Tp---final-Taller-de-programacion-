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
    } else if (command == "/meditar") {
        cmd.set_message_type(MSG_MEDITATE);
    } else if (command == "/fundar-clan") {
        std::string clan_name;
        if (!(ss >> clan_name)) {
            throw std::invalid_argument("Uso: /fundar-clan <nombre>");
        }

        cmd.set_message_type(MSG_FOUND_CLAN);
        cmd.set_target_name(clan_name);
    } else {
        throw std::invalid_argument("Comando desconocido: " + command);
    }

    return cmd;
}
