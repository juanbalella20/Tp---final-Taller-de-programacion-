#ifndef CLIENT_NETWORK_RECEIVER_THREAD_H_
#define CLIENT_NETWORK_RECEIVER_THREAD_H_

#include "../common/thread.h"
#include "../common/queue.h"
#include "../common/gameMsg.h"
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
