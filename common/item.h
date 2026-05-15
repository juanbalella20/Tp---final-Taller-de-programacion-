#ifndef ITEM_H
#define ITEM_H

#include <string>

class Player; 

class Item {
protected:
    int id;
    std::string name;
    int price;

public:
    Item(int id, const std::string& name, int price);
    virtual ~Item() = default;

    int getId() const;

    std::string getName() const;
    
    int getPrice() const;

    virtual void use_item(Player& player) = 0;
};

#endif