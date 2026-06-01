#include "clientApp.h"
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
    cmd.set_message_type(MSG_REGISTER);
    cmd.set_player_name(player_name_);
    cmd.set_race(race_);
    cmd.set_class(class_);
    protocol.send_command(cmd);
}

void ClientApp::run() {
    Socket skt(host_.c_str(), port_.c_str());
    ClientProtocol protocol(std::move(skt));
    initialize_connection(protocol);

    NetworkSenderThread sender(protocol, sendingQueue);
    NetworkReceiverThread receiver(protocol, receivingQueue);
    ClientGUI gui(sendingQueue, receivingQueue, player_name_, race_);

    sender.start();
    receiver.start();
    // gui.start();

    // gui.join();

    gui.run();
    
    sendingQueue.close();
    receivingQueue.close();

    protocol.shutdown();

    sender.join();
    receiver.join();
}
