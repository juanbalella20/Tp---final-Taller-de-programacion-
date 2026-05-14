#ifndef SERIALIZER_H
#define SERIALIZER_H
#include "../common/clientCmd.h"
#include <vector>
class Serializer{

    public:
    std::vector<uint8_t> serialize_cmd(const ClientCmd& cmd);

};
#endif  // SERIALIZER_H