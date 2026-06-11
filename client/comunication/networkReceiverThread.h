#ifndef CLIENT_NETWORK_RECEIVER_THREAD_H_
#define CLIENT_NETWORK_RECEIVER_THREAD_H_

#include "../../common/utility/thread.h"
#include "../../common/utility/queue.h"
#include "../../common/commands/gameMsg.h"
#include "clientProtocol.h"

class NetworkReceiverThread : public Thread {
private:
    ClientProtocol& protocol;
    Queue<GameMsg>& receiving;

public:
    NetworkReceiverThread(ClientProtocol& protocol, Queue<GameMsg>& receiving);
    void run() override;
};

#endif  // CLIENT_NETWORK_RECEIVER_THREAD_H_
