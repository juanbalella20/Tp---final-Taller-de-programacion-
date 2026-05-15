#include "serverProtocol.h"

ServerProtocol::ServerProtocol(Socket&& socket) : socket(std::move(socket)), deserializer() {}

bool ServerProtocol::receive_command(ClientCmd& cmd) {
    uint8_t type;
    if (socket.recvall(&type, sizeof(type)) == 0) return false;

    uint16_t payload_len_be;
    if (socket.recvall(&payload_len_be, sizeof(payload_len_be)) == 0) return false;

    uint16_t payload_len = ntohs(payload_len_be);

    std::vector<uint8_t> payload(payload_len);
    if (payload_len > 0) {
        if (socket.recvall(payload.data(), payload_len) == 0) return false;
    }

    deserializer.deserialize_cmd(type, payload, cmd);
    return true;
}

void ServerProtocol::send_event(const GameMsg& msg) {}

void ServerProtocol::disconnect() {
    socket.shutdown(SHUT_RDWR);
    socket.close();
}
