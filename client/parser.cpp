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
    } else {
        throw std::invalid_argument("Comando desconocido: " + command);
    }

    return cmd;
}
