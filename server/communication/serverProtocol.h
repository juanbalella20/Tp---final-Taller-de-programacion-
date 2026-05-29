#ifndef SERVER_SRC_SERVER_PROTOCOL_H_
#define SERVER_SRC_SERVER_PROTOCOL_H_

#include "serializer/serverDeserializer.h"
#include "../../common/commands/clientCmd.h"
#include "../../common/commands/gameMsg.h"
#include "../../common/socket/socket.h"
#include "serializer/serverSerializer.h"

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
