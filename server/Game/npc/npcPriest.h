#ifndef NPC_FRIENDLY_PRIEST_H_
#define NPC_FRIENDLY_PRIEST_H_

#include "npcFriendly.h"

class Player;

class NPCpriest : public NPCfriendly {
    private:
        int pos_x;
        int pos_y;
        /*
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
        */
    public:
        // an npc_id is generated and a name
        NPCpriest();
        /* interacts with player:
         * - reseructs player
         * - heals player
         * - sells item to player
         */
        NPCpriest(int x, int y) : pos_x(x), pos_y(y) {}
        NPCpriest(NPCpriest&&) = default;
        NPCpriest& operator=(NPCpriest&&) = default;
        NPCpriest(const NPCpriest&) = delete;
        NPCpriest& operator=(const NPCpriest&) = delete;

        int get_coord_x() const { return pos_x; }
        int get_coord_y() const { return pos_y; }

        void interact(Player& player, Command cmd) override {}

};

#endif 
