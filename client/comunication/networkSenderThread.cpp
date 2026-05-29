#include "networkSenderThread.h"

NetworkSenderThread::NetworkSenderThread(ClientProtocol& protocol, Queue<ClientCmd>& sendingQueue)
    : protocol(protocol), sendingQueue(sendingQueue) {}

void NetworkSenderThread::run() {
    try {
        while (should_keep_running()) {
            ClientCmd cmd = sendingQueue.pop();
            protocol.send_command(cmd);
        }
    } catch (const ClosedQueue&) {}
}
