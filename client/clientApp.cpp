#include "clientApp.h"

#include <utility>

#include "interface/screen_manager.h"
#include "interface/auth_session.h"

ClientApp::ClientApp(const std::string& host, const std::string& port):
        host_(host), port_(port) {}

void ClientApp::run() {
    // 1) Pantallas pre-juego (config + login/registro). La conexion al server
    //    esta DIFERIDA: el ScreenManager/las screens crean el ClientProtocol
    //    recien al primer intento de auth y lo dejan en la AuthSession.
    ScreenManager screen_manager(host_, port_);
    AuthSession session;
    ScreenState result = screen_manager.run(session);

    if (result != ScreenState::GAME || !session.authenticated || !session.protocol) {
        return;  // el usuario cerro antes de autenticarse.
    }

    // 2) Auth OK: tomamos la conexion ya abierta de la sesion y arrancamos los
    //    threads de red sobre ella. El world snapshot llega por el socket DESPUES
    //    del MSG_CONFIRM_SESSION y lo lee directamente el NetworkReceiverThread
    //    (no hay nada que re-encolar).
    std::unique_ptr<ClientProtocol> protocol = std::move(session.protocol);
    session.password.clear();  // descartar la contrasena en claro tras autenticar.

    NetworkSenderThread sender(*protocol, sendingQueue);
    NetworkReceiverThread receiver(*protocol, receivingQueue);

    // 3) Juego sobre el MISMO window/renderer/font del ScreenManager.
    ClientGUI gui(screen_manager.get_window(), screen_manager.get_renderer(),
                  screen_manager.get_font(), sendingQueue, receivingQueue,
                  session.name, session.race);

    sender.start();
    receiver.start();

    gui.run();

    sendingQueue.close();
    receivingQueue.close();

    protocol->shutdown();

    sender.join();
    receiver.join();
}
