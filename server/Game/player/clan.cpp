#include "clan.h"

Clan::Clan(const std::string& founder_name, const std::string& clan_name) :
    founder_name(founder_name),
    clan_name(clan_name) {

    members.push_back(founder_name);
}

