#ifndef COMMON_CLIENT_CMD_H_
#define COMMON_CLIENT_CMD_H_

#include <cstdint>
#include <string>


#include "protocol_constants.h"

class ClientCmd {
 private:
    MessageType message_type;
    uint32_t client_id;
    std::string player_name;
    EntityType target_type;
    std::string target_name;
    std::string race;
    std::string klass;
    std::string item_id;
    Direction direction;

 public:
    ClientCmd();

    //set id
    //get id
    void set_message_type(MessageType type);
    MessageType get_message_type() const;

    void set_client_id(uint32_t id);
    uint32_t get_client_id() const;

    void set_player_name(const std::string& name);
    const std::string& get_player_name() const;

    void set_target_type(EntityType type);
    EntityType get_target_type() const;

    void set_target_name(const std::string& name);
    const std::string& get_target_name() const;

    void set_race(const std::string& race_value);
    const std::string& get_race() const;

    void set_class(const std::string& class_value);
    const std::string& get_class() const;

    void set_direction(Direction dir);
    Direction get_direction() const;
};

#endif  // COMMON_CLIENT_CMD_H_
