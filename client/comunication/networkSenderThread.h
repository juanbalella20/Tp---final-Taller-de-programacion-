#ifndef CLIENT_NETWORK_SENDER_THREAD_H_
#define CLIENT_NETWORK_SENDER_THREAD_H_

#include "../../common/utility/thread.h"
#include "../../common/utility/queue.h"
#include "../../common/commands/clientCmd.h"
#include "clientProtocol.h"

class NetworkSenderThread : public Thread {
private:
    ClientProtocol& protocol;
    Queue<ClientCmd>& sendingQueue;

public:
    NetworkSenderThread(ClientProtocol& protocol, Queue<ClientCmd>& sendingQueue);
    void run() override;
};

#endif  // CLIENT_NETWORK_SENDER_THREAD_H_
