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
    // Fuerza el cierre para que run() termine.
    try { receiver.stop(); } catch (...) {}
    try { sender.stop(); } catch (...) {}
    try { sending_queue.close(); } catch (...) {}
    try { protocol.disconnect(); } catch (...) {}
    // Espera que el thread propio (run) termine antes de destruir miembros.
    try { Thread::join(); } catch (...) {}
    try { receiver.join(); } catch (...) {}
    try { sender.join(); } catch (...) {}
}

void ClientHandler::run() {
    bool registrated = false;
    try {
        client_registry_monitor.add_client(client_id, sending_queue);
        registrated = true;

        receiver.start();
        sender.start();

        receiver.join();
        sender.stop();
        try {
            sending_queue.close();
        } catch (...) {}
        sender.join();
    } catch (...) {}

    if (registrated) {
        try {
            client_registry_monitor.remove_client(client_id);
        } catch (...) {}
    }
}
