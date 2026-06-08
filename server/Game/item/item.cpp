#include "item.h"

uint64_t Item::next_uid = 1;  // 0 se reserva como "sin item"

Item::Item(const std::string& id, const std::string& name, int price)
    : id(id), name(name), price(price), uid(next_uid++) {}

std::string Item::get_id() const {
    return id;
}

uint64_t Item::get_uid() const {
    return uid;
}

std::string Item::getName() const {
    return name;
}

int Item::getPrice() const {
    return price;
}