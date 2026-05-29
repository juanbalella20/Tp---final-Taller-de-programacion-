#ifndef NPC_INFO_H
#define NPC_INFO_H

#include <string>

// Snapshot de un NPC tal como lo recibe el cliente.
// El server arma una lista de estos cada vez que el estado de los NPCs cambia
// (spawn, muerte, respawn, movimiento) y la envia en MSG_NPCS_SNAPSHOT
struct NpcInfo {
    std::string type;
    std::string name;   
    int x;
    int y;

    NpcInfo() : type(), name(), x(0), y(0) {}
    NpcInfo(std::string type, std::string name, int x, int y)
        : type(std::move(type)), name(std::move(name)), x(x), y(y) {}
};

#endif
