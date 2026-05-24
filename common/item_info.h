#ifndef ITEM_INFO_H
#define ITEM_INFO_H

#include <string>

struct ItemInfo {
    std::string id;
    std::string name;
    int price;

    ItemInfo() : id(""), name(""), price(0) {}
    ItemInfo(const std::string& id, const std::string& name, int price)
        : id(id), name(name), price(price) {}

    const std::string& get_id() const { return id; }
    const std::string& get_name() const { return name; }
    int get_price() const { return price; }

    void set_id(const std::string& new_id) { id = new_id; }
    void set_name(const std::string& new_name) { name = new_name; }
    void set_price(int new_price) { price = new_price; }
};

#endif  // ITEM_INFO_H
