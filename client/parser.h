#ifndef PARSER_H
#define PARSER_H
#include "../common/commands/clientCmd.h"
#include "../common/constants/protocol_constants.h"

class Parser {
 private:
    ClientCmd parse_private(const std::string& command, std::istringstream& ss);
    ClientCmd parse_no_payload(MessageType type);
    ClientCmd parse_item_cmd(MessageType type, std::istringstream& ss, const std::string& correct_use);
    ClientCmd parse_gold_cmd(MessageType type, std::istringstream& ss, const std::string& correct_use);
    ClientCmd parse_name_cmd(MessageType type, std::istringstream& ss, const std::string& correct_use);

    bool health_command(const std::string& command, ClientCmd& cmd);
    bool cheat_command(const std::string& command, ClientCmd& cmd);
    bool item_related_command(const std::string& command, ClientCmd& cmd, std::istringstream& ss);
    bool clan_related_command(const std::string& command, ClientCmd& cmd, std::istringstream& ss);
    bool gold_related_command(const std::string& command, ClientCmd& cmd, std::istringstream& ss);
    bool teleport_command(const std::string& command, ClientCmd& cmd, std::istringstream& ss);
    ClientCmd parse_tp_zone(MessageType type, std::string zone);
 public:
    static ClientCmd parse(const std::string& input);
    ClientCmd parse_chat(const std::string& input);
};
#endif  // PARSER_H
