#include "client_handler.h"
#include "../../common/utility/queue.h"
#include "../../common/socket/socket.h"
#include "../../common/commands/clientCmd.h"

#include <utility>

ClientHandler::ClientHandler(uint32_t client_id,
                             Socket&& socket,
                             Queue<ClientCmd>& receiving_queue,
                             ClientRegistryMonitor& client_registry_monitor)
    : client_id(client_id),
      protocol(std::move(socket)),
      receiving_queue(receiving_queue),
      client_registry_monitor(client_registry_monitor),
      receiver(client_id, protocol, receiving_queue),
      sender(protocol, sending_queue) {}

ClientHandler::~ClientHandler() {
    stop();
    try { sending_queue.close(); } catch (...) {}
    try { protocol.disconnect(); } catch (...) {}
    try { receiver.join(); } catch (...) {}
    try { sender.join(); } catch (...) {}
}

void ClientHandler::start() {
    client_registry_monitor.add_client(client_id, sending_queue);
    receiver.start();
    sender.start();
}

void ClientHandler::stop() {
    try { receiver.stop(); } catch (...) {}
    try { sender.stop(); } catch (...) {}
}

void ClientHandler::join() {
    receiver.join();

    // Cuando el receiver termina, el cliente se desconectó: notificar al gameloop.
    try {
        std::string name = client_registry_monitor.get_name(client_id);
        if (!name.empty()) {
            ClientCmd logout;
            logout.set_message_type(MSG_LOGOUT);
            logout.set_client_id(client_id);
            logout.set_player_name(name);
            receiving_queue.push(logout);
        }
    } catch (...) {}

    try { client_registry_monitor.remove_client(client_id); } catch (...) {}

    try { sending_queue.close(); } catch (...) {}
    sender.join();
}

bool ClientHandler::is_alive() const {
    return receiver.is_alive() || sender.is_alive();
}
