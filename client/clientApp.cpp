#include "clientApp.h"
#include "parser.h"
#include <iostream>
#include <stdexcept>
#include <utility>

ClientApp::ClientApp(const std::string& host,
                     const std::string& port,
                     const std::string& player_name,
                     const std::string& race,
                     const std::string& klass):
        host_(host), port_(port), player_name_(player_name), race_(race), class_(klass) {}

void ClientApp::initialize_connection(ClientProtocol& protocol) {
    ClientCmd cmd;
    //Apriori lo voy a pensar como registro sin persistencia
    cmd.set_message_type(MSG_REGISTER);
    cmd.set_player_name(player_name_);
    cmd.set_race(race_);
    cmd.set_class(class_);
   
    protocol.send_command(cmd);
}


void ClientApp::game_loop(ClientProtocol& protocol) {
    std::string input;
    while (std::getline(std::cin, input)) {
        if (input == "q")
            break;
        try {
            ClientCmd cmd = Parser::parse(input);
            protocol.send_command(cmd);
        } catch (const std::invalid_argument& e) {
            std::cout << e.what() << "\n";
        }
    }
}

void ClientApp::run() {
    Socket skt(host_.c_str(), port_.c_str());
    ClientProtocol protocol(std::move(skt));
    initialize_connection(protocol);
    game_loop(protocol);
}
