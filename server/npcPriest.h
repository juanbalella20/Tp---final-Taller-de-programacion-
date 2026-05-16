#ifndef NPC_FRIENDLY_PRIEST_H_
#define NPC_FRIENDLY_PRIEST_H_

#include "npcFriendly.h"

class NPCpriest : public NPCfriendly {
    private:
        // store can be a class
        Store store;

        // gets priest store
        Store get_store();
        // adds or delete an item from the store
        void update_store();
        // revives player
        void revive(Player player);
        // heals player
        void heal(Player player);
        // sells item to player
        void sell_item(Item item, Player player, int cantidad);

    public:
        // an npc_id is generated and a name
        NPCpriest();
        /* interacts with player:
         * - reseructs player
         * - heals player
         * - sells item to player
         */
        void interact(Player player, Command cmd) override;

};

#endif 
