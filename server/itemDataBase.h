#ifndef ITEM_DATA_BASE_H_
#define ITEM_DATA_BASE_H_

#include <map>
#include <string>


class ItemDataBase {
    private:

        std::map<std::string, Item> potions;
        std::map<std::string, Item> weapons;
        std::map<std::string, Item> armour;
        std::map<std::string, Item> magic_items;
    
    public:
        Item get_random_potion();
        Item get_random_weapon();
        Item get_random_armour();
        Item get_random_magic();
 
        Item get_random_item();

        std::map<std::string, Item> get_potions();
        std::map<std::string, Item> get_weapons();
        std::map<std::string, Item> get_armour();
        std::map<std::string, Item> get_magic_items();

}

#endif 
