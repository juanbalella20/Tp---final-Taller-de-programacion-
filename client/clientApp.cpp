#include "client_app.h"

#include <iostream>
#include <stdexcept>
#include <utility>

ClientApp::ClientApp(const std::string& host, const std::string& port): host_(host), port_(port) {}

ClientProtocol ClientApp::initialize_connection() {
    Socket skt(host_.c_str(), port_.c_str());
    ClientProtocol protocol(std::move(skt));
    printer_.print_ask_name();
    std::string name;
    if (reader_.read_line(name)) {
        ClientCmd cmd;
        cmd.set_type(ClientCmdType::REGISTER_NAME);
        cmd.set_player_name(name);
        protocol.send_command(cmd);
        return protocol;
    } else {
        throw std::runtime_error("Failed to read player name");
    }
}

bool ClientApp::handle_command(ClientProtocol& protocol, const std::string& line) {
    try {
        ClientCmd cmd = parser_.parse(line);
        if (cmd.type() == ClientCmdType::EXIT) {
            protocol.disconnect();
            return false;
        }
        if (cmd.type() == ClientCmdType::READ) {
            for (uint16_t i = 0; i < cmd.read_count(); i++) {
                printer_.print_event(protocol.receive_event());
            }
            return true;
        }
        protocol.send_command(cmd);
    } catch (const std::exception& e) {
        printer_.print_error(e.what());
    }
    return true;
}

void ClientApp::run() {
    ClientProtocol protocol = initialize_connection();
    std::string line;
    while (reader_.read_line(line) && handle_command(protocol, line)) {}
}
