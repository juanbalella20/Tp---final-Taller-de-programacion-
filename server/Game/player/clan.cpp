#include "clan.h"

Clan::Clan(const std::string& founder_name, const std::string& clan_name) :
    founder_name(founder_name),
    clan_name(clan_name) {

    members.push_back(founder_name);
}

bool Clan::join(const std::string& player_name) {
    if (player_name == founder_name) {
        return false;
    }
    members.push_back(player_name);
    return true;
}