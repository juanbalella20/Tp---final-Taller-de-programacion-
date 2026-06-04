#ifndef CLAN_H_
#define CLAN_H_

#include <string>
#include <vector>

class Clan {

private:
    std::string founder_name;
    std::string clan_name;

    std::vector<std::string> members;

public:
    Clan(const std::string& founder_name, const std::string& clan_name);

    bool join(const std::string& player_name);

    void leave(const std::string& clan_name);

    void kick(const std::string& member, const std::string& clan_name);

};

#endif
