#include "clientProtocol.h"

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