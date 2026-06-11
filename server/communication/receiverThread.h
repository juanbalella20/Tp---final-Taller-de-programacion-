#ifndef SERVER_SRC_SERVER_RECEIVER_H_
#define SERVER_SRC_SERVER_RECEIVER_H_

#include "../../common/utility/thread.h"
#include "../../common/utility/queue.h"
#include "../../common/commands/clientCmd.h"
// #include "../common_src/client_cmd.h"  // Missing file: common_src/client_cmd.h
// #include "server_protocol.h"  // Missing file: server/server_protocol.h
#include "serverProtocol.h"

class ReceiverThread : public Thread {
 private:
    uint32_t client_id;
    ServerProtocol& protocol;
    Queue<ClientCmd>& receiving_queue;

 public:
    ReceiverThread(uint32_t client_id, ServerProtocol& protocol, Queue<ClientCmd>& receiving_queue);
    ~ReceiverThread() override;
    void run() override;
};

#endif  // SERVER_SRC_SERVER_RECEIVER_H_
