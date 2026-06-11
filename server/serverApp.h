#ifndef SERVER_SRC_SERVER_APP_H_
#define SERVER_SRC_SERVER_APP_H_


#include "../common/utility/queue.h"
#include "acceptor.h"
#include "clientRegistryMonitor.h"
#include "gameloop.h"

class ServerApp {
    private:
    std::string port_;
    Queue<ClientCmd> receiving_queue;
    ClientRegistryMonitor client_registry_monitor;
    Acceptor acceptor;
    GameLoop game_loop;

    public:
    explicit ServerApp(std::string port);
    ~ServerApp();
    void run();
};

#endif  // SERVER_SRC_SERVER_APP_H_
