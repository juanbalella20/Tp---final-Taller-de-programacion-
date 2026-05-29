#ifndef ITEM_FLOOR_INFO_H
#define ITEM_FLOOR_INFO_H

#include <string>

// Snapshot de un item tirado en el piso tal como lo recibe el cliente.
// El server arma una lista de estos cada vez que el estado de los items en el
// piso cambia (drop, pickup, spawn inicial, gold drop) y la envia en
// MSG_ITEMS_SNAPSHOT.
struct ItemFloorInfo {
    std::string type;   // ej: "espada", "gold"
    int x;
    int y;
    
    ItemFloorInfo() : type(), x(0), y(0) {}
    ItemFloorInfo(std::string type, int x, int y)
        : type(std::move(type)), x(x), y(y) {}
};

#endif