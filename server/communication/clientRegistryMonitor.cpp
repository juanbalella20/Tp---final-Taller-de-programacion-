#include "clientRegistryMonitor.h"

#include "../../common/utility/queue.h"

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

void ClientRegistryMonitor::notify_client(uint32_t client_id, const GameMsg& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = registers.find(client_id);
    if (it == registers.end()) {
        throw std::runtime_error("client_id is not registered");
    }
    it->second.sending_queue.get().push(msg);
}

void ClientRegistryMonitor::notify_client_by_name(const std::string& player_name, const GameMsg& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& entry : registers) {
        if (entry.second.player_name == player_name) {
            try {
                entry.second.sending_queue.get().push(msg);
            } catch (const ClosedQueue&) {}
            return;
        }
    }
    // Si no se encuentra, era un NPC — se ignora silenciosamente
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

void ClientRegistryMonitor::notify_clients_less_client(uint32_t client_id, const GameMsg& msg) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& entry: registers) {
        if (entry.first == client_id) continue;
        try {
            entry.second.sending_queue.get().push(msg);
        } catch (const ClosedQueue&) {
            // Cliente desconectado: se ignora.
        }
    }
}
std::vector<std::pair<uint32_t, std::string>> ClientRegistryMonitor::get_active_clients() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<std::pair<uint32_t, std::string>> result;
    result.reserve(registers.size());
    for (const auto& entry : registers) {
        result.emplace_back(entry.first, entry.second.player_name);
    }
    return result;
}
