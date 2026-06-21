#include "serverApp.h"

#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
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
//antes en el tp de threads el server se cortaba con una q, ahora ni idea de como se deberia cortar
    char c = '\0';
    while (std::cin.get(c)) {
        if (c == 'q') {
            break;
        }
    }

    acceptor.shutdown();
    game_loop.stop();
    try {
        receiving_queue.close();
    } catch (...) {}
    acceptor.join();
    game_loop.join();
}
