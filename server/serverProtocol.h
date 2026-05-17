#ifndef SERVER_SRC_SERVER_PROTOCOL_H_
#define SERVER_SRC_SERVER_PROTOCOL_H_

#include "serverDeserializer.h"
#include "../common/clientCmd.h"
#include "../common/gameMsg.h"
#include "../common/socket.h"
#include "serverSerializer.h"

#include <arpa/inet.h>
#include <cstdint>
#include <vector>

class ServerProtocol {
 private:
    Socket socket;
    ServerDeserializer deserializer;
    ServerSerializer serializer;
  

 public:
    explicit ServerProtocol(Socket&& socket);

    bool receive_command(ClientCmd& cmd);
    void send_event(const GameMsg& msg);

    void disconnect();
};

#endif  // SERVER_SRC_SERVER_PROTOCOL_H_
