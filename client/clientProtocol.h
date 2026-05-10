#ifndef CLIENT_SRC_CLIENT_PROTOCOL_H_
#define CLIENT_SRC_CLIENT_PROTOCOL_H_
#include "../common/socket.h"

class ClientProtocol {
private:
    Socket socket;
    //ClientSerializer serializer;
    //ClientDeserializer deserializer;
 public:
    ClientProtocol();
    ~ClientProtocol();
    void initialize_connection();
    
};






#endif  // CLIENT_SRC_CLIENT_PROTOCOL_H_