#ifndef PARSER_H
#define PARSER_H
#include "../common/clientCmd.h"
#include "../common/protocol_constants.h"

class Parser {
 public:
    static ClientCmd parse(const std::string& input);
    static ClientCmd parse_chat(const std::string& input);
};
#endif  // PARSER_H
