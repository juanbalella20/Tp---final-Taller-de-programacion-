#ifndef NPC_FRIENDLY_PRIEST_H_
#define NPC_FRIENDLY_PRIEST_H_

#include "npcFriendly.h"
#include "../../../common/info/item_info.h"
#include "../item/item.h"
#include "../item/item_catalog.h"

#include <vector>
#include <memory>

class Player;

class NPCpriest : public NPCfriendly {
    private:
        int pos_x;
        int pos_y;
        
        std::vector<std::unique_ptr<Item>> store_items;
        ItemCatalog catalog;

        void init_store();
        void sell_item(Player& player, const std::string& item_id);
        void resurrect(Player& player);
        void heal(Player& player);

    public:
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

        std::vector<ItemInfo> list_items() const;
        void interact(Player& player, Command cmd) override;

};

#endif 
