#ifndef CLIENT_APP_H
#define CLIENT_APP_H
#include <string>
#include "clientProtocol.h"
#include "clientCmd.h"
#include "gameMsg.h"
#include "queue.h"
#include "networkSenderThread.h"
#include "networkReceiverThread.h"
#include "clientGUI.h"

// Orquesta el cliente: lee host/puerto de config.toml, corre el ScreenManager
// (config -> login/registro, con conexion DIFERIDA) y, si el usuario se
// autentica, arranca los threads de red sobre la conexion ya abierta y entra al
// juego (ClientGUI) reutilizando el mismo window/renderer/font.
class ClientApp {
 private:
    std::string host_;
    std::string port_;

    Queue<ClientCmd> sendingQueue;
    Queue<GameMsg> receivingQueue;

 public:
    // host/port se leen de config.toml en clientmain; ./client va sin argumentos.
    ClientApp(const std::string& host, const std::string& port);
    void run();
};
#endif  // CLIENT_APP_H
