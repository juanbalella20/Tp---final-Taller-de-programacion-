#ifndef PLAYER_H_
#define PLAYER_H_

#include "entity.h"
#include "entity.h"
#include "player_race.h"
#include "player_class.h"
#include "../inventory.h"

#include <string>
#include <memory>

enum class PlayerStatus { ALIVE, DEAD };

class Player : public Entity {

private:
    std::string name;
    uint32_t gold;
    uint32_t lives;
    uint32_t experience;
    uint32_t mana;
    int id_clan;
    int level;
    int coord_x;
    int coord_y;
    bool meditating;
    PlayerStatus status;
    std::shared_ptr<Item> equipped_item;

    PlayerRace player_race;
    PlayerClass player_class;
    Inventory player_inventory;

    uint32_t max_life();

    uint32_t max_mana();

    void level_up();

public:
    Player(const std::string name, PlayerRace& player_race, PlayerClass& player_class);

    void add_item(std::unique_ptr<Item> item);

    void drop_item(Item* item);

    void equip_item(std::string item_id);

    void unequip_item(Item item);

    void use_object(Item item);

    void attack(Entity& target, int target_x, int target_y);

    void revive();

    void heal_life(const int healthy_life);

    void heal_mana(const int healthy_mana);

    void heal(const int healthy_life, const int healthy_mana);

    void add_gold(const int extra_gold);

    bool give_gold(const int amount);

    uint32_t get_gold() const;

    std::string get_name() const override;

    int get_coord_x() const;

    int get_coord_y() const;

    void update_position(const int x, const int y);

    const Inventory& get_inventory() const;
    bool is_ghost() const;
    bool is_dead() const override;
    
    void set_ghost();

    bool is_meditating() const;

    void change_meditation();

    void stop_meditation();

    bool can_meditate() const;

    uint32_t get_lives() const;

    int get_level() const;

    int get_clan_id() const;

    int damage_attack();

    void receive_damage(int damage) override;

    void add_experience(int exp);

    void check_level_up();

    uint32_t get_xp() const;

    uint32_t get_mana() const;

    const std::string& get_race_name() const;
};

#endif
