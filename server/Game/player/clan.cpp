#include "clan.h"

#include <algorithm>
#include <utility>

Clan::Clan(const std::string& founder_name, const std::string& clan_name) :
    founder_name(founder_name),
    clan_name(clan_name),
    clan_review("") {

    members.push_back(founder_name);
}

Clan::Clan(const std::string& founder_name, const std::string& clan_name,
           const std::string& clan_review, std::vector<std::string> members,
           std::vector<std::string> banned_players) :
    founder_name(founder_name),
    clan_name(clan_name),
    clan_review(clan_review),
    members(std::move(members)),
    banned_players(std::move(banned_players)) {}

bool Clan::join_request(const std::string& player_name) {
    if (player_name == founder_name) {
        return false;
    }

    auto it = std::find(banned_players.begin(), banned_players.end(), player_name);
    
    if (it != banned_players.end()) {
        return false;
    }

    std::string incoming = "Jugador " + player_name + " solicitó unirse al clan " + clan_name + "\n";
    clan_review += incoming;

    return true;
}

std::string Clan::review() {
    return clan_review;
}

bool Clan::is_founder(const std::string& player_name) { return player_name == founder_name; }

void Clan::accept_join_request(const std::string& player_name) {
    members.push_back(player_name);
}

bool Clan::leave(const std::string& player_name) {
    if (player_name == founder_name) {
        return false;
    }

    auto it = std::find(members.begin(), members.end(), player_name);
    
    if (it != members.end()) {
        members.erase(it);
        return true;
    }
    
    return false;
}

bool Clan::joined(const std::string& player_name) const {
    if (player_name == founder_name) {
        return true;
    }

    auto member = std::find_if(members.begin(), members.end(), 
        [&player_name](auto& m) { return m == player_name; }
    );

    if (member == members.end()) {
        return false;
    }
    return true;
}

bool Clan::kick(const std::string& member) {
    return leave(member);
}

bool Clan::ban(const std::string& member) {
    if (leave(member)) {
        banned_players.push_back(member);
        return true;
    }
    return false;
}

bool Clan::same_clan(const std::string& player_name_1, const std::string& player_name_2) {
    auto player1 = std::find(members.begin(), members.end(), player_name_1);
    auto player2 = std::find(members.begin(), members.end(), player_name_2);
    
    if (player1 != members.end() && player2 != members.end()) {
        return true;
    }
    return false;
}
