#ifndef ITEM_INFO_H
#define ITEM_INFO_H

#include <string>
#include <cstdint>

struct ItemInfo {
    std::string id;
    std::string name;
    int price;
    // Tipo del item para el HUD: 0=arma, 1=armadura, 2=casco, 3=escudo, 4=otro.
    uint8_t type;

    ItemInfo() : id(""), name(""), price(0), type(4) {}
    ItemInfo(const std::string& id, const std::string& name, int price, uint8_t type = 4)
        : id(id), name(name), price(price), type(type) {}

    const std::string& get_id() const { return id; }
    const std::string& get_name() const { return name; }
    int get_price() const { return price; }
    uint8_t get_type() const { return type; }

    void set_id(const std::string& new_id) { id = new_id; }
    void set_name(const std::string& new_name) { name = new_name; }
    void set_price(int new_price) { price = new_price; }
    void set_type(uint8_t new_type) { type = new_type; }
};

#endif  // ITEM_INFO_H
