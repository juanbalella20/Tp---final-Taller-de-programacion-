#ifndef SERVER_SRC_SERVER_PROTOCOL_H_
#define SERVER_SRC_SERVER_PROTOCOL_H_

//#include "server_serializer.h"
//#include "server_deserializer.h"
#include "../common/clientCmd.h"
#include "../common/gamMsg.h"
#include "../common/socket.h"

#include <cstdint>
#include <vector>

class ServerProtocol {
 private:
    Socket socket;
    //ServerSerializer serializer;
    //ServerDeserializer deserializer;
    std::vector<uint8_t> recv_register_payload();
    std::vector<uint8_t> recv_defeat_payload();

 public:
    explicit ServerProtocol(Socket&& socket);

    bool receive_command(ClientCmd& cmd);
    void send_event(const GameMsg& msg);

    void disconnect();
};

#endif  // SERVER_SRC_SERVER_PROTOCOL_H_
