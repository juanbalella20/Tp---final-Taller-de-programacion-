#ifndef ITEM_H
#define ITEM_H

#include <string>

class Player; 

class Item {
protected:
    std::string id;
    std::string name;
    int price;

public:
    Item(const std::string& id, const std::string& name, int price);
    virtual ~Item() = default;

    std::string getId() const;

    std::string getName() const;
    
    int getPrice() const;

    virtual void use_item(Player& player) = 0;
};

#endif