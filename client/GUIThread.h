#ifndef CLIENT_GUI_THREAD_H_
#define CLIENT_GUI_THREAD_H_

#include "../common/thread.h"
#include "../common/queue.h"
#include "../common/clientCmd.h"
#include "../common/gameMsg.h"

class GUIThread : public Thread {
private:
    Queue<ClientCmd>& outgoing;
    Queue<GameMsg>& receiving;

public:
    GUIThread(Queue<ClientCmd>& outgoing, Queue<GameMsg>& receiving);
    void run() override;
};

#endif  // CLIENT_GUI_THREAD_H_
