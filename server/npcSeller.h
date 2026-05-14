#ifndef NPC_FRIENDLY_SELLER_H_
#define NPC_FRIENDLY_SELLER_H_

#include "npcFriendly.h"

class NPCseller: public NPCfriendly {
    private:
        Store store;

        // gets store
        Store get_store();
        // adds an item from the store
        void add_item_store(Item item);   
        // deletes an item from the store
        void delete_item_store(Item item);
        // sells item to player
        Item sell_item();
        // buys item from player
        void buy_item(Item item, Player player);


    public:
        // an npc_id is generated and a name
        NPCseller();
        /* interacts with player:
         * - buys item from player
         * - sells item to player
         */
        void interact(Player player_tag, Command cmd) override;

};

#endif 
