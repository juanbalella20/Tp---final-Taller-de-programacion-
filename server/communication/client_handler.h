#ifndef SERVER_SRC_SERVER_CLIENT_HANDLER_H_
#define SERVER_SRC_SERVER_CLIENT_HANDLER_H_

#include "../../common/utility/queue.h"
#include "../../common/utility/thread.h"
#include "receiverThread.h"
#include "senderThread.h"
#include "clientRegistryMonitor.h"

class ClientHandler : public Thread {
 private:
    uint32_t client_id;
    ServerProtocol protocol;
    Queue<GameMsg> sending_queue;
    Queue<ClientCmd>& receiving_queue;
    ClientRegistryMonitor& client_registry_monitor;
    ReceiverThread receiver;
    SenderThread sender;

 public:
    ClientHandler(uint32_t client_id,
                  Socket&& socket,
                  Queue<ClientCmd>& receiving_queue,
                  ClientRegistryMonitor& client_registry_monitor);
    ~ClientHandler() override;
    void run() override;  // lógica que antes estaba en start()
};

#endif  // SERVER_SRC_SERVER_CLIENT_HANDLER_H_
