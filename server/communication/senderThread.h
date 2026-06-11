#ifndef SERVER_SRC_SERVER_SENDER_H_
#define SERVER_SRC_SERVER_SENDER_H_

#include "../../common/utility/thread.h"
#include "../../common/utility/queue.h"
#include "../../common/commands/gameMsg.h"
// #include "server_protocol.h"  // Missing file: server/server_protocol.h
#include "serverProtocol.h"

class SenderThread : public Thread {
 private:
    ServerProtocol& protocol;
    Queue<GameMsg>& sending_queue;

 public:
    SenderThread(ServerProtocol& protocol, Queue<GameMsg>& sending_queue);
    ~SenderThread() override;
    void run() override;
};

#endif  // SERVER_SRC_SERVER_SENDER_H_
