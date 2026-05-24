#include "item.h"

Item::Item(const std::string& id, const std::string& name, int price)
    : id(id), name(name), price(price) {}

std::string Item::getId() const {
    return id;
}

std::string Item::getName() const {
    return name;
}

int Item::getPrice() const {
    return price;
}