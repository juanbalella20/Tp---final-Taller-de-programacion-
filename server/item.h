#ifndef ITEM_H
#define ITEM_H

#include <string>

class Entity;

class Item {
protected:
    std::string id;
    std::string name;
    int price;

public:
    Item(const std::string& id, const std::string& name, int price);
    virtual ~Item() = default;

    std::string get_id() const;
    

    std::string getName() const;
    
    int getPrice() const;

    virtual void use_item(Entity& target, int attacker_x, int attacker_y, int target_x, int target_y) = 0;
};

#endif