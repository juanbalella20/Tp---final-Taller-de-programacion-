#ifndef NPC_FRIENDLY_BANKER_H_
#define NPC_FRIENDLY_BANKER_H_


#include "npcFriendly.h"
#include <map>
#include <vector>
#include <memory>
#include "../item/item.h"
#include "../../../common/info/item_info.h"

class Player;

class NPCbanker: public NPCfriendly {
    private:
        int pos_x;
        int pos_y;

        std::map<std::string, std::vector<std::unique_ptr<Item>>> item_bank;
        std::map<std::string, int> gold_bank;

        void deposit_item(Player& player, const std::string& item_id);
        void deposit_gold(Player& player, int amount);
        void retire_item(Player& player, const std::string& item_id);
        void retire_gold(Player& player, int amount);

    public:
        NPCbanker(int x, int y);
        int get_coord_x() const;
        int get_coord_y() const; 

        std::vector<ItemInfo> list_bank_items(const std::string& player_name) const;
        int get_bank_gold(const std::string& player_name) const;

        void interact(Player& player, Command cmd) override;

        NPCbanker(NPCbanker&&) = default;
        NPCbanker& operator=(NPCbanker&&) = default;
        NPCbanker(const NPCbanker&) = delete;
        NPCbanker& operator=(const NPCbanker&) = delete;

};

#endif 
