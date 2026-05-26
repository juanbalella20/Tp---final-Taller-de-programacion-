#ifndef NPC_FRIENDLY_SELLER_H_
#define NPC_FRIENDLY_SELLER_H_

#include "npcFriendly.h"

#include "item_info.h"
#include "item.h"

#include <vector>
#include <memory>
#include <string>

class Player;
 
class NPCseller : public NPCfriendly {
private:
    int pos_x;
    int pos_y;
    std::vector<std::unique_ptr<Item>> store_items;
 
    void init_store();
 
public:
    NPCseller(int x, int y);
 
    int get_coord_x() const;
    int get_coord_y() const;
 
    std::vector<ItemInfo> list_items() const;
    bool interact(Player& player, const std::string& item_id);
};


#endif 
