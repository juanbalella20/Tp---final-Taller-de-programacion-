#ifndef SERVER_SRC_SERVER_ACCEPTOR_H_
#define SERVER_SRC_SERVER_ACCEPTOR_H_

#include "../common/socket.h"
#include "../common/thread.h"
#include "../common/queue.h"
#include "../common/clientCmd.h"
#include "client_handler.h"
#include <cstdint>
#include <memory>
#include <vector>
#include "clientRegistryMonitor.h"

class Acceptor : public Thread {
    private:
    Socket socket;
    std::vector<std::unique_ptr<ClientHandler>> clients;
    Queue<ClientCmd>& receiving_queue;
    ClientRegistryMonitor& client_registry_monitor;
    uint32_t next_client_id;
    void clear();
    void reap();


    public:
    explicit Acceptor(Socket&& socket,
                      Queue<ClientCmd>& receiving_queue,
                      ClientRegistryMonitor& client_registry_monitor);
    ~Acceptor() override;
    void run() override;
    
};

#endif  // SERVER_SRC_SERVER_ACCEPTOR_H_
