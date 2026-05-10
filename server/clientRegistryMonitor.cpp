#include "clientRegistryMonitor.h"

#include "../common/queue.h"

#include <stdexcept>

void ClientRegistryMonitor::add_client(uint32_t client_id,
                                       std::reference_wrapper<Queue<GameMsg>> sending_queue) {
    std::lock_guard<std::mutex> lock(mtx);
    registers.insert_or_assign(client_id, ClientRecord("", sending_queue));
}

void ClientRegistryMonitor::assign_name(uint32_t client_id,
                                        const std::string& player_name) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = registers.find(client_id);
    if (it == registers.end()) {
        throw std::runtime_error("client_id is not registered");
    }

    
    it->second.player_name = player_name;
}

std::string ClientRegistryMonitor::get_name(uint32_t client_id) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = registers.find(client_id);
    if (it == registers.end()) {
        throw std::runtime_error("client_id is not registered");
    }
    return it->second.player_name;
}

void ClientRegistryMonitor::remove_client(uint32_t client_id) {
    std::lock_guard<std::mutex> lock(mtx);
    registers.erase(client_id);
}


void ClientRegistryMonitor::notify_clients(const GameMsg& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& entry: registers) {
        try {
            entry.second.sending_queue.get().push(msg);
        } catch (const ClosedQueue&) {
            // Cliente desconectado: se ignora.
        }
    }
}
