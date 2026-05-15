#include "item.h"

Item::Item(int id, const std::string& name, int price)
    : id(id), name(name), price(price) {}

int Item::getId() const {
    return id;
}

std::string Item::getName() const {
    return name;
}

int Item::getPrice() const {
    return price;
}