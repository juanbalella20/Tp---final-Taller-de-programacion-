#ifndef SERVER_SRC_GAMELOOP_H_
#define SERVER_SRC_GAMELOOP_H_

#include "../common/clientCmd.h"
#include "../common/queue.h"
#include "../common/thread.h"
#include "clientRegistryMonitor.h"
#include "game_map.h"

class GameLoop : public Thread {
 public:
    GameLoop(Queue<ClientCmd>& receiving_queue, ClientRegistryMonitor& client_registry_monitor);
    void run() override;

 private:
    Queue<ClientCmd>& receiving_queue;
    ClientRegistryMonitor& client_registry_monitor;
    GameMap game_map;
    void load_maps();
    void load_world();
   // Guarda el ultimo NPC seleccionado por cada cliente (client_id -> {x, y})
    std::unordered_map<uint32_t, std::pair<int,int>> selected_npc;

    void handle_attack(const ClientCmd& cmd);
    void handle_meditate(const ClientCmd& cmd);
    //void handle_resurrect(const ClientCmd& cmd);
};

#endif  // SERVER_SRC_GAMELOOP_H_
