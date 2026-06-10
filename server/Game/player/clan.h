#ifndef CLAN_H_
#define CLAN_H_

#include <string>
#include <vector>

class Clan {

private:
    std::string founder_name;
    std::string clan_name;
    std::string clan_review;

    std::vector<std::string> members;
    std::vector<std::string> banned_players;

public:
    Clan(const std::string& founder_name, const std::string& clan_name);

    // Reconstruye un clan completo desde disco (persistencia). Restaura founder,
    // nombre, review y las listas de miembros/baneados tal cual estaban.
    Clan(const std::string& founder_name, const std::string& clan_name,
         const std::string& clan_review, std::vector<std::string> members,
         std::vector<std::string> banned_players);

    bool join_request(const std::string& player_name);

    std::string review();

    bool is_founder(const std::string& player_name);

    void accept_join_request(const std::string& player_name);

    bool leave(const std::string& player_name);

    bool joined(const std::string& player_name) const;

    std::string& get_name() { return clan_name; }
    const std::string& get_name() const { return clan_name; }

    // --- Accesores para persistencia (clans.dat) ---
    const std::string& get_founder() const { return founder_name; }
    const std::string& get_review() const { return clan_review; }
    const std::vector<std::string>& get_members() const { return members; }
    const std::vector<std::string>& get_banned() const { return banned_players; }

    bool kick(const std::string& member);

    bool ban(const std::string& member);

    bool same_clan(const std::string& player_name_1, const std::string& player_name_2);

};

#endif
