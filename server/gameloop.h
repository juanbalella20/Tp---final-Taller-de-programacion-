#ifndef SERVER_SRC_GAMELOOP_H_
#define SERVER_SRC_GAMELOOP_H_

#include "../common/clientCmd.h"
#include "../common/queue.h"
#include "../common/thread.h"
#include "clientRegistryMonitor.h"

class GameLoop : public Thread {
 public:
    GameLoop(Queue<ClientCmd>& receiving_queue, ClientRegistryMonitor& client_registry_monitor);
    void run() override;

 private:
    Queue<ClientCmd>& receiving_queue;
    ClientRegistryMonitor& client_registry_monitor;
};

#endif  // SERVER_SRC_GAMELOOP_H_
