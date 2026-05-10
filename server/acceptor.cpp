#include "acceptor.h"
#include <algorithm>
#include <memory>
#include <sys/socket.h>
#include <utility>

Acceptor::Acceptor(Socket&& socket,
                   Queue<ClientCmd>& receiving_queue,
                   ClientRegistryMonitor& client_registry_monitor):
        socket(std::move(socket)),
        clients(),
        receiving_queue(receiving_queue),
        client_registry_monitor(client_registry_monitor),
        next_client_id(1) {}

Acceptor::~Acceptor() {
    stop();
    try {
        socket.shutdown(SHUT_RDWR);
    } catch (...) {}
    try {
        socket.close();
    } catch (...) {}
    if (is_alive()) {
        try {
            join();
        } catch (...) {}
    }
}

void Acceptor::run(){

    while (should_keep_running()){
        try{
            Socket socket_client= socket.accept();
            const uint32_t client_id = next_client_id++;
            auto client_handler = std::make_unique<ClientHandler>(
                    client_id, std::move(socket_client), receiving_queue, client_registry_monitor);

            clients.push_back(std::move(client_handler));
            clients.back()->start();
            
            reap();
        }

        catch (const std::exception& e){
            if (should_keep_running()) {
                std::cerr << "Error accepting client: " << e.what() << std::endl;
            }
        }
    }

    clear();
}




void Acceptor::clear() {
    // Al limpiar el contenedor, cada ClientHandler libera receiver/sender en su destructor.
    clients.clear();
}
