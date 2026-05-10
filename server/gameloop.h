#ifndef SERVER_SRC_GAMELOOP_H_
#define SERVER_SRC_GAMELOOP_H_

class GameLoop : public Thread {
 public:
    GameLoop(Queue<ClientCmd>& receiving_queue, ClientRegistryMonitor& client_registry_monitor);
    void run();
};

#endif  // SERVER_SRC_GAMELOOP_H_