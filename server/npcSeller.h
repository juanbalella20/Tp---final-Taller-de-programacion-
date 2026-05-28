#ifndef NPC_FRIENDLY_SELLER_H_
#define NPC_FRIENDLY_SELLER_H_

#include "npcFriendly.h"
#include "npc.h"

#include "../common/item_info.h"
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

    NPCseller(NPCseller&&) = default;
    NPCseller& operator=(NPCseller&&) = default;
 
    NPCseller(const NPCseller&) = delete;
    NPCseller& operator=(const NPCseller&) = delete;

    int get_coord_x() const;
    int get_coord_y() const;
 
    std::vector<ItemInfo> list_items() const;
    // interact maneja ACTION_SELL, ACTION_BUY segun cmd.action
    void interact(Player& player, Command cmd) override;

};


#endif 
