#ifndef ENTITY_H_
#define ENTITY_H_

#include <string>

class Entity {
public:
    virtual ~Entity() = default;

    virtual std::string get_name() const = 0;
    virtual void receive_damage(int damage) = 0;
    virtual bool is_dead() const = 0;
};

#endif
