#include "senderThread.h"

SenderThread::SenderThread(ServerProtocol& protocol, Queue<GameMsg>& sending_queue)
    : protocol(protocol), sending_queue(sending_queue) {}

SenderThread::~SenderThread() {
    stop();
    try {
        sending_queue.close();
    } catch (...) {}
    try {
        join();
    } catch (...) {}
}

void SenderThread::run() {
    while (should_keep_running()) {
        try {
            GameMsg msg = sending_queue.pop();
            protocol.send_event(msg);
        } catch (const ClosedQueue&) {
            break;
        }
    }
}
