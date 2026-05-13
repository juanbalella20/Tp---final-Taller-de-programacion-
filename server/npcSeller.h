#ifndef NPC_FRIENDLY_SELLER_H_
#define NPC_FRIENDLY_SELLER_H_

// Comerciante -> nombre seller deberia cambiar

#include "npcFriendly.h"

class NPCseller: public NPCfriendly {
    private:
        Store store;

        // gets store
        Store get_store();
        // adds or delete an item from the store
        void update_store()       
        // sells item to player
        Item sell_item();
        // buys item from player
        void buy_item();


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
