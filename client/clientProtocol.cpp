#include "clientProtocol.h"
#include "common/gameMsg.h"
#include "common/protocol_constants.h"
#include <arpa/inet.h>

ClientProtocol::ClientProtocol(Socket&& socket) : socket(std::move(socket)),serializer() {}

/*
ClientProtocol::~ClientProtocol() {
    try {
        socket.shutdown(SHUT_RDWR);
    } catch (...) {}
    try {
        socket.close();
    } catch (...) {}
}
*/
void ClientProtocol::send_command(const ClientCmd& cmd) {
    std::vector<uint8_t> bytes = serializer.serialize_cmd(cmd);
    socket.sendall(bytes.data(), bytes.size());
}

bool ClientProtocol::receive_event(GameMsg& msg) {
    uint8_t type;
    if (socket.recvall(&type, sizeof(type)) == 0) return false;

    uint16_t payload_len_be;
    if (socket.recvall(&payload_len_be, sizeof(payload_len_be)) == 0) return false;

    uint16_t payload_len = ntohs(payload_len_be);

    std::vector<uint8_t> payload(payload_len);
    if (payload_len > 0) {
        if (socket.recvall(payload.data(), payload_len) == 0) return false;
    }

    deserializer.deserialize_cmd(type, payload, msg);
    return true;
}
