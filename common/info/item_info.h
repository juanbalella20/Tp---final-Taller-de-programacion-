#ifndef ITEM_INFO_H
#define ITEM_INFO_H

#include <string>
#include <cstdint>

struct ItemInfo {
    // id de TIPO (p.ej. "espada"): resuelve imagen/nombre. Compartido por copias.
    std::string id;
    std::string name;
    int price;
    // Tipo del item para el HUD: 0=arma, 1=armadura, 2=casco, 3=escudo, 4=otro.
    uint8_t type;
    // uid de INSTANCIA: unico por item. Distingue dos copias del mismo tipo para
    // equipar/resaltar la correcta. 0 = sin asignar (items que no vienen del
    // inventario del server, p.ej. catalogo del vendedor).
    uint64_t uid;

    ItemInfo() : id(""), name(""), price(0), type(4), uid(0) {}
    ItemInfo(const std::string& id, const std::string& name, int price,
             uint8_t type = 4, uint64_t uid = 0)
        : id(id), name(name), price(price), type(type), uid(uid) {}

    const std::string& get_id() const { return id; }
    const std::string& get_name() const { return name; }
    int get_price() const { return price; }
    uint8_t get_type() const { return type; }
    uint64_t get_uid() const { return uid; }

    void set_type(uint8_t new_type) { type = new_type; }
};

#endif  // ITEM_INFO_H
