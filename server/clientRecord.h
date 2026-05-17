#ifndef SERVER_SRC_SERVER_CLIENT_RECORD_H_
#define SERVER_SRC_SERVER_CLIENT_RECORD_H_

#include <functional>
#include "../common/gameMsg.h"
#include <string>
#include "../common/queue.h"

struct ClientRecord {
    std::string player_name;
    std::reference_wrapper<Queue<GameMsg>> sending_queue;

    ClientRecord(const std::string& player_name,
                 std::reference_wrapper<Queue<GameMsg>> sending_queue)
        : player_name(player_name), sending_queue(sending_queue) {}
};

#endif  // SERVER_SRC_SERVER_CLIENT_RECORD_H_
