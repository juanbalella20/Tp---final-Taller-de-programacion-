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

    bool join_request(const std::string& player_name);

    std::string review();

    bool is_founder(const std::string& player_name);

    void accept_join_request(const std::string& player_name);

    bool leave(const std::string& player_name);

    bool joined(const std::string& player_name);

    std::string& get_name() { return clan_name; }

    bool kick(const std::string& member);

    bool ban(const std::string& member);

    bool same_clan(const std::string& player_name_1, const std::string& player_name_2);

};

#endif
