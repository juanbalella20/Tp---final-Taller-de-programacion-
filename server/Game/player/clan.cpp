#include "clan.h"

Clan::Clan(const std::string& founder_name, const std::string& clan_name) :
    founder_name(founder_name),
    clan_name(clan_name),
    clan_review("No hay novedades para el clan " + clan_name) {

    members.push_back(founder_name);
}

bool Clan::join(const std::string& player_name) {
    if (player_name == founder_name) {
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