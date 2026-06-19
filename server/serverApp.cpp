#include "serverApp.h"

#include <sys/socket.h>
#include <iostream>

ServerApp::ServerApp(std::string port):
    port_(std::move(port)),
    receiving_queue(),
    client_registry_monitor(),
    acceptor(Socket(port_.c_str()), receiving_queue, client_registry_monitor),
    game_loop(receiving_queue, client_registry_monitor) {}

ServerApp::~ServerApp() {
    try {
        receiving_queue.close();
    } catch (...) {
    }
}

void ServerApp::run() {
    game_loop.start();
    acceptor.start();
    char c = '\0';
    while (std::cin.get(c)) {
        if (c == 'q') {
            break;
        }
    }

    acceptor.stop();
    game_loop.stop();
    try {
        receiving_queue.close();
    } catch (...) {}
    acceptor.join();
    game_loop.join();
}
