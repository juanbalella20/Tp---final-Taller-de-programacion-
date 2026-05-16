#ifndef CLIENT_APP_H
#define CLIENT_APP_H
#include <string>
#include "clientProtocol.h"
#include "../common/clientCmd.h"
#include "parser.h"


class ClientApp {
 private:
   std::string host_;
   std::string port_;
   std::string player_name_;
   std::string race_;
   std::string class_;
   //ClientPrinter printer_;
   //ClientReader reader_;
   void initialize_connection(ClientProtocol& protocol);
   void game_loop(ClientProtocol& protocol);

 public:
    ClientApp(const std::string& host,
              const std::string& port,
              const std::string& player_name,
              const std::string& race,
              const std::string& klass);
    void run();
};
#endif  // CLIENT_APP_H
