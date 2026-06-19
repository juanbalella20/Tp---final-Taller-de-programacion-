#ifndef CLIENT_SRC_CLIENT_PROTOCOL_H_
#define CLIENT_SRC_CLIENT_PROTOCOL_H_
#include "../../common/socket/socket.h"
#include "../../common/commands/clientCmd.h"
#include <vector>
#include "serializer.h"
#include "deserializer.h"
class ClientProtocol {
private:
    Socket socket;
    Serializer serializer;
    ClientDeserializer deserializer;
 public:
    ClientProtocol(Socket&& socket);
    ~ClientProtocol();
    void initialize_connection();
    void send_command(const ClientCmd& cmd);
    bool receive_event(GameMsg& msg);
    void shutdown();
    
};

#endif  // CLIENT_SRC_CLIENT_PROTOCOL_H_