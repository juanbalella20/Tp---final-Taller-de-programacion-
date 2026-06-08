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
#include "clan.h"

#include <string>
#include <memory>
#include <vector>

class Player : public Entity {

    // Los estados manipulan el estado interno del jugador (vida, inventario,
    // transición de estado) al ejecutar las acciones que gobiernan.
    friend class AliveState;
    friend class GhostState;
    friend class MeditateState;

private:
    std::string name;
    uint32_t gold;
    uint32_t lives;
    uint32_t experience;
    uint32_t mana;
    int id_clan;
    int coord_x;
    int coord_y;
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
    // to_meditate: pasa a meditar.
    void to_ghost();
    void to_alive();
    void to_meditate();

public:
    Player(const std::string name, PlayerRace& player_race, PlayerClass& player_class);

    void add_item(std::unique_ptr<Item> item);

    void drop_item(Item* item);

    // Saca todos los items del inventario al morir y los devuelve.
    std::vector<std::unique_ptr<Item>> drop_inventory();

    // Equipa el item (por id) en el slot que corresponde a su tipo.
    // Para armas funciona como toggle: si ya está equipada, la desequipa.
    void equip_item(std::string item_id);

    // ¿El jugador tiene un arma equipada?
    bool has_weapon_equipped() const;

    // Ids de todos los items equipados (arma + defensas). Para resaltar el inventario.
    std::vector<std::string> get_equipped_ids() const;

    // Defensa total que aportan los items de defensa equipados.
    int calculate_defense();

    void unequip_item(Item item);

    void use_object(Item item);

    DamageOutcome attack(Entity& target, int target_x, int target_y);

    // Lanza el hechizo del item equipado sobre uno mismo (auto-cast, p.ej.
    // curación con la flauta élfica). Usa la propia celda como target.
    void cast_on_self();

    // Suma a este jugador (atacante) la XP por golpear/matar a un target.
    bool ganar_xp(int dano, int nivel_target, bool murio, int vida_max_target);

    void revive();

    void heal_life(const int healthy_life);

    void heal_mana(const int healthy_mana);

    // Resta maná (no baja de 0). Usado por el cheat /mana para testear /meditar.
    void lose_mana(const int amount);

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

    // Deja de meditar (no-op si no estaba meditando). Cualquier acción debe
    // llamarlo: meditar se interrumpe ante cualquier interacción.
    void stop_meditation();

    // Alterna meditación (comando /meditar). Devuelve true si tras el toggle el
    // jugador quedó meditando, false si dejó de meditar o no pudo empezar.
    bool toggle_meditation();

    bool can_meditate() const;

    // ¿La clase puede usar magia (hechizos/báculos)? Solo el guerrero no puede.
    bool can_cast() const;

    // Avance de tiempo del game loop: delega en el estado. Devuelve true si el
    // jugador está meditando (maná actualizado, hay que notificar al cliente).
    bool tick(double seconds);

    // Recupera maná por meditación: FClaseMeditacion * Inteligencia * segundos,
    // tope en max_mana(). Lo invoca MeditateState desde tick().
    void recover_meditation_mana(double seconds);

    uint32_t get_lives() const;

    int get_level() const;

    uint32_t max_xp() const;

    // Fair play: ¿es newbie (nivel <= 12)?
    bool is_newbie() const;

    // Fair play: ¿puede atacar a un jugador de otro_nivel?
    bool can_attack_level(int other_level) const;

    int get_clan_id() const;

    int damage_attack();

    DamageOutcome receive_damage(int damage, Player& atacante, bool is_critical) override;

    void add_experience(int exp);

    bool check_level_up();

    uint32_t get_xp() const;

    uint32_t get_mana() const;

    const std::string& get_race_name() const;

    // --- Identidad e I/O de persistencia ---

    // Identidad de raza/clase como enum del dominio (para serializar a disco).
    RaceType get_race_id() const;
    ClassType get_class_id() const;

    // Restaura el estado mutable de un jugador cargado de disco. Setea los
    // campos que el constructor calcula y para los que no hay setter (gold,
    // lives, mana, experience, level, id_clan). La posicion se setea aparte con
    // update_position; el estado ghost con set_ghost(). race/class/name son
    // inmutables (van por el constructor).
    void restore(uint32_t gold, uint32_t lives, uint32_t mana,
                 uint32_t experience, int level, int id_clan);
};

#endif
