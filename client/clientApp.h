#ifndef CLIENT_APP_H
#define CLIENT_APP_H
#include <string>
#include "clientProtocol.h"

class ClientApp {
 private:
   std::string host_;
   std::string port_;
   //ClientPrinter printer_;
   //ClientReader reader_;
   //Parser parser_;
   ClientProtocol initialize_connection();
   bool handle_command(ClientProtocol& protocol, const std::string& line);

 public:
    ClientApp(const std::string& host, const std::string& port);
    void run();
};
#endif  // CLIENT_APP_H
