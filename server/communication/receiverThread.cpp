#include "receiverThread.h"

#include <iostream>

ReceiverThread::ReceiverThread(uint32_t client_id,
                               ServerProtocol& protocol,
                               Queue<ClientCmd>& receiving_queue)
    : client_id(client_id), protocol(protocol), receiving_queue(receiving_queue) {}

ReceiverThread::~ReceiverThread() {
    stop();
    try {
        join();
    } catch (...) {}
}

void ReceiverThread::run() {
    while (should_keep_running()) {
        try {
            ClientCmd cmd;
            if (!protocol.receive_command(cmd)) {
                break;
            }
            cmd.set_client_id(client_id);
            receiving_queue.push(cmd);
        } catch (const std::exception& e) {
            if (should_keep_running()) {
                std::cerr << "Error receiving client command: " << e.what() << std::endl;
            }
            break;
        }
    }
}
