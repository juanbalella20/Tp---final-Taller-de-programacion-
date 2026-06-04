#ifndef PLAYER_H_
#define PLAYER_H_

#include "entity.h"
#include "entity.h"
#include "player_race.h"
#include "player_class.h"
#include "../inventory.h"
#include "../defense_set.h"
#include "level.h"
#include "player_state.h"

#include <string>
#include <memory>
#include <vector>

class Player : public Entity {

    // Los estados manipulan el estado interno del jugador (vida, inventario,
    // transición de estado) al ejecutar las acciones que gobiernan.
    friend class AliveState;
    friend class GhostState;

private:
    std::string name;
    uint32_t gold;
    uint32_t lives;
    uint32_t experience;
    uint32_t mana;
    int id_clan;
    int coord_x;
    int coord_y;
    bool meditating;
    std::unique_ptr<PlayerState> state;
    std::shared_ptr<Item> equipped_item;

    PlayerRace player_race;
    PlayerClass player_class;
    Inventory player_inventory;
    DefenseSet defense_set;
    Level level;

    uint32_t max_life();

    uint32_t max_mana();

    // Transiciones de estado internas (las disparan los propios estados).
    // to_ghost: pasa a fantasma (muerto). to_alive: vuelve a vivo.
    void to_ghost();
    void to_alive();

public:
    Player(const std::string name, PlayerRace& player_race, PlayerClass& player_class);

    void add_item(std::unique_ptr<Item> item);

    void drop_item(Item* item);

    // Saca todos los items del inventario al morir y los devuelve.
    std::vector<std::unique_ptr<Item>> drop_inventory();

    // Equipa el item (por id) en el slot que corresponde a su tipo.
    void equip_item(std::string item_id);

    // Defensa total que aportan los items de defensa equipados.
    int calculate_defense();

    void unequip_item(Item item);

    void use_object(Item item);

    int attack(Entity& target, int target_x, int target_y);

    // Suma a este jugador (atacante) la XP por golpear/matar a un target.
    void ganar_xp(int dano, int nivel_target, bool murio, int vida_max_target);

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

    // Todos los items del jugador (inventario + defensa equipada), para mostrar.
    std::vector<Item*> get_all_items() const;
    bool is_ghost() const;
    bool is_dead() const override;
    bool can_interact() const;
    
    void set_ghost();

    bool is_meditating() const;

    void change_meditation();

    void stop_meditation();

    bool can_meditate() const;

    uint32_t get_lives() const;

    int get_level() const;

    // Fair play: ¿es newbie (nivel <= 12)?
    bool is_newbie() const;

    // Fair play: ¿puede atacar a un jugador de otro_nivel?
    bool can_attack_level(int other_level) const;

    int get_clan_id() const;

    int damage_attack();

    int receive_damage(int damage, Player& atacante, bool is_critical) override;

    void add_experience(int exp);

    void check_level_up();

    uint32_t get_xp() const;

    uint32_t get_mana() const;

    const std::string& get_race_name() const;
};

#endif
