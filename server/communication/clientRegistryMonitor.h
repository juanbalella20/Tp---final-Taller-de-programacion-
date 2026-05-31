#ifndef SERVER_MONITOR_REGISTRAR_CLIENT_H_
#define SERVER_MONITOR_REGISTRAR_CLIENT_H_

#include "clientRecord.h"

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>

class ClientRegistryMonitor {
 private:
    std::mutex mtx;
    std::unordered_map<uint32_t, ClientRecord> registers;

 public:
    void add_client(uint32_t client_id,
                      std::reference_wrapper<Queue<GameMsg>> sending_queue);
    void assign_name(uint32_t client_id, const std::string& player_name);
    std::string get_name(uint32_t client_id);
    void remove_client(uint32_t client_id);
    void notify_client(uint32_t client_id, const GameMsg& msg);
    void notify_clients(const GameMsg& msg);
    void notify_clients_less_client(uint32_t client_id, const GameMsg& msg);
};

#endif  // SERVER_MONITOR_REGISTRAR_CLIENT_H_
