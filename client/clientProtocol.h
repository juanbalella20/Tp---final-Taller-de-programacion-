#ifndef CLIENT_SRC_CLIENT_PROTOCOL_H_
#define CLIENT_SRC_CLIENT_PROTOCOL_H_
#include "../common/socket.h"
#include "../common/clientCmd.h"

class ClientProtocol {
private:
    Socket socket;
    //ClientSerializer serializer;
    //ClientDeserializer deserializer;
 public:
    ClientProtocol(Socket&& socket);
    ~ClientProtocol();
    void initialize_connection();
    void send_command(const ClientCmd& cmd);

    
};






#endif  // CLIENT_SRC_CLIENT_PROTOCOL_H_