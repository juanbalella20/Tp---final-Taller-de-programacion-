#include "networkReceiverThread.h"

NetworkReceiverThread::NetworkReceiverThread(ClientProtocol& protocol, Queue<GameMsg>& receiving)
    : protocol(protocol), receiving(receiving) {}

void NetworkReceiverThread::run() {
    try {
        while (should_keep_running()) {
            GameMsg msg(0);
            if (!protocol.receive_event(msg)) break;
            receiving.push(msg);
        }
    } catch (const ClosedQueue&) {}
}
