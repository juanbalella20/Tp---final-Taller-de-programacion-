#include "player.h"
#include "../../game_exceptions.h"
#include "alive_state.h"
#include "ghost_state.h"
#include "meditate_state.h"

#include <cmath>

Player::Player(const std::string name, PlayerRace& player_race, PlayerClass& player_class):
    name(name),
    state(std::make_unique<AliveState>()),
    equipped_item(nullptr),
    player_race(player_race),
    player_class(player_class),
    player_inventory(),
    level(16) {

    lives = max_life();
    gold = 10000;
    experience = 0;
    mana = max_mana();
}

void Player::to_ghost() {
    state = std::make_unique<GhostState>();
}

void Player::to_alive() {
    state = std::make_unique<AliveState>();
}

void Player::to_meditate() {
    state = std::make_unique<MeditateState>();
}

const Inventory& Player::get_inventory() const {
    return player_inventory;
}

std::vector<Item*> Player::get_all_items() const {
    // El Inventory es dueño de todos los items (armas y defensa). El DefenseSet
    // solo referencia cuáles de ellos están equipados.
    return player_inventory.get_items();
}

uint32_t Player::max_life() {
    // VidaMax = Constitución * FClaseVida * FRazaVida * Nivel
    return player_race.race_constitution() * player_class.class_life_factor() * player_race.race_life_factor() * level.get_number();
}

uint32_t Player::max_mana() {
    // ManaMax = Inteligencia * FClaseMana * FRazaMana * Nivel
    return (player_race.race_inteligence() + player_class.class_inteligence()) * player_class.class_mana_factor() * player_race.race_mana_factor() * level.get_number();
}


void Player::add_item(std::unique_ptr<Item> item) {
    // Tomar/recibir un item es una acción: interrumpe la meditación.
    // (Al spawnear el jugador está vivo, así que es un no-op.)
    stop_meditation();
    player_inventory.add_item(std::move(item));
}

/* TODO: implement ITEM */


void Player::drop_item(Item* item) {
    player_inventory.drop_item(item);
}

std::vector<std::unique_ptr<Item>> Player::drop_inventory() {
    // Al morir caen todos los items: el Inventory es dueño de armas y defensa.
    // El DefenseSet solo referenciaba algunos, así que se limpia.
    defense_set.clear();
    return player_inventory.drop_all();
}

/*

void Player::equip_item(Item item) {
    if (equipped_item == nullptr) {
        player_inventory.unequip_item(item);
        equipped_item = item;
    } else {
        player_inventory.equip_item(equipped_item);
        player_inventory.unequip_item(item);
        equipped_item = item;
    }
}

void Player::unequip_item(Item item) {
    if (equipped_item == nullptr) {
        return;
    }

    if (equipped_item == item) {
        player_inventory.equip_item(item);
        equipped_item = nullptr;
    }
}

void Player::use_object(Item item) {
    item.use_item();
}
*/



void Player::revive() {
    state->revive(*this);
}

void Player::heal_life(const int healthy_life) {
    if (lives + healthy_life < max_life()) {
        lives += healthy_life;
    } else {
        lives = max_life();
    }
}

void Player::heal_max_life() {
    heal_life(max_life());
}

void Player::heal_max_mana() {
    heal_mana(max_mana());
}

void Player::set_inmortal() {
    inmortal = true;
}
void Player::set_inf_mana() {
    inf_mana = true;
}


void Player::heal_mana(const int healthy_mana) {
    if (mana + healthy_mana < max_mana()) {
        mana += healthy_mana;
    } else {
        mana = max_mana();
    }
}

void Player::lose_mana(const int amount) {
    if (inf_mana) return;
    if (amount <= 0) return;
    if (static_cast<uint32_t>(amount) >= mana) {
        mana = 0;
    } else {
        mana -= static_cast<uint32_t>(amount);
    }
}

void Player::heal(const int healthy_life, const int healthy_mana) {
    heal_life(healthy_life);
    heal_mana(healthy_mana);
}

bool Player::regen_life(double percent) {
    if (is_dead()) return false;
    uint32_t max = max_life();
    if (lives >= max) return false;  // ya al maximo: nada que notificar
    // Cura un porcentaje de la vida MAXIMA
    int amount = static_cast<int>(std::ceil(max * percent));
    if (amount <= 0) return false;
    heal_life(amount);
    return true;
}

bool Player::regen_mana(double percent) {
    if (is_dead()) return false;
    uint32_t max = max_mana();
    if (mana >= max) return false;  // ya al maximo: nada que notificar
    // Cura un porcentaje del mana máximo
    int amount = static_cast<int>(std::ceil(max * percent));
    if (amount <= 0) return false;
    heal_mana(amount);
    return true;
}

void Player::add_gold(const int extra_gold) {
    // Tomar/recibir oro es una acción: interrumpe la meditación.
    stop_meditation();
    gold += extra_gold;
}

bool Player::give_gold(const int amount) {
    if (gold >= amount) {
        gold -= amount;
        return true;
    }

    return false;
}

uint32_t Player::get_gold() const {
    return gold;
}

std::string Player::get_name() const {
    return name;
}

int Player::get_coord_x() const {
    return coord_x;
}

int Player::get_coord_y() const {
    return coord_y;
}

void Player::update_position(const int x, const int y) {
    // Moverse es una acción: interrumpe la meditación.
    stop_meditation();
    coord_x = x;
    coord_y = y;
}

bool Player::is_ghost() const {
    return state->is_ghost();
}

bool Player::is_dead() const {
    return state->is_ghost();
}

bool Player::can_interact() const {
    return state->can_interact();
}

void Player::set_ghost() {
    to_ghost();
}

void Player::stop_meditation() {
    state->stop_meditation(*this);
}

bool Player::toggle_meditation() {
    return state->toggle_meditation(*this);
}

bool Player::can_meditate() const {
    return !state->is_ghost() && player_class.class_can_meditate();
}

bool Player::can_cast() const {
    // El guerrero es la única clase que no puede usar magia. Reusa el mismo
    // criterio que la meditación (factor de meditación 0 => guerrero).
    return player_class.class_can_meditate();
}

bool Player::tick(double seconds) {
    return state->tick(*this, seconds);
}

void Player::recover_meditation_mana(double seconds) {
    // Mana = FClaseMeditacion * Inteligencia * segundos (tope en max_mana()).
    int inteligence = player_race.race_inteligence() + player_class.class_inteligence();
    double gained = player_class.class_meditation_factor() * inteligence * seconds;
    heal_mana(static_cast<int>(gained));
}

uint32_t Player::get_lives() const {
    return lives;
}

uint32_t Player::get_xp() const {
    return experience;
}

uint32_t Player::get_mana() const {
    return mana;
}

uint32_t Player::get_max_life() {
    return max_life();
}

uint32_t Player::get_max_mana() {
    return max_mana();
}
 
int Player::get_level() const {
    return level.get_number();
}

bool Player::is_newbie() const {
    return level.is_newbie();
}

bool Player::can_attack_level(int other_level) const {
    return level.can_attack_level(other_level);
}
 
int Player::damage_attack() {
    // Daño = Fuerza * rand(DañoArmaMin, DañoArmaMax)
    // TODO: cuando Item este definido, usar el arma equipada
    return player_race.race_strength() + player_class.class_strength();
}
 
DamageOutcome Player::receive_damage(int damage, Player& atacante, bool is_critical) {
    if (inmortal) return {0,0,false,false,nullptr};
    return state->receive_damage(*this, damage, atacante, is_critical);
}

DamageOutcome Player::receive_npc_damage(int damage, bool is_critical) {
    if (inmortal) return {0,0,false,false,nullptr};
    return state->receive_npc_damage(*this, damage, is_critical);
}

DamageOutcome Player::attack(Entity& target, int target_x, int target_y) {
    return state->attack(*this, target, target_x, target_y);
}

void Player::cast_on_self() {
    // Auto-cast: el target es uno mismo. El item equipado decide el efecto
    // (la flauta élfica cura; consume maná). Usa la propia celda como target.
    stop_meditation();
    player_inventory.use_equipped(*this, *this, coord_x, coord_y,
                                  coord_x, coord_y, false);
}

bool Player::use_consumable(uint64_t item_uid) {
    // Tomar una poción es una acción: interrumpe la meditación.
    stop_meditation();
    return player_inventory.use_consumable(item_uid, *this);
}

bool Player::ganar_xp(int dano, int nivel_target, bool murio, int vida_max_target) {
    experience += level.xp_per_attack(dano, nivel_target);
    if (murio)
        experience += level.xp_per_kill(vida_max_target, nivel_target);
    return check_level_up();
}
 
void Player::add_experience(int exp) {
    experience += exp;
}

void Player::equip_item(uint64_t item_uid) {
    // Equipar es una acción: interrumpe la meditación.
    stop_meditation();
    // Busca la INSTANCIA exacta (por uid) y la equipa en el slot que corresponde
    // a su tipo. El Inventory sigue siendo dueño; el DefenseSet solo referencia.
    Item* item = player_inventory.find_item(item_uid);
    if (item == nullptr) return;

    switch (item->get_type()) {
        case ItemType::WEAPON:
        case ItemType::MAGIC:
            // Toggle: si esta instancia ya está equipada, la desequipa; si no, la
            // equipa. Arma y báculo comparten slot, así que equipar uno desplaza
            // al otro.
            if (player_inventory.is_equipped(item_uid)) {
                player_inventory.unequip_item();
            } else {
                player_inventory.equip_item(item_uid);
            }
            break;
        case ItemType::ARMOR:
            if (defense_set.is_equipped(item_uid)) {
                defense_set.unequip_armadura();
            } else {
                defense_set.equip_armadura(static_cast<DefenseItem*>(item));
            }
            break;
        case ItemType::HELMET:
            if (defense_set.is_equipped(item_uid)) {
                defense_set.unequip_casco();
            } else {
                defense_set.equip_casco(static_cast<DefenseItem*>(item));
            }
            break;
        case ItemType::SHIELD:
            if (defense_set.is_equipped(item_uid)) {
                defense_set.unequip_escudo();
            } else {
                defense_set.equip_escudo(static_cast<DefenseItem*>(item));
            }
            break;
        default:
            break;
    }
}

void Player::equip_item_by_type(const std::string& type_id) {
    // Reconstrucción desde persistencia: busca la primera instancia de ese tipo
    // y delega en equip_item(uid). Al cargar hay a lo sumo un item marcado como
    // equipado por slot, así que tomar la primera coincidencia es correcto.
    for (Item* item : player_inventory.get_items()) {
        if (item->get_id() == type_id) {
            equip_item(item->get_uid());
            return;
        }
    }
}

bool Player::has_weapon_equipped() const {
    return player_inventory.has_weapon_equipped();
}

std::vector<uint64_t> Player::get_equipped_uids() const {
    std::vector<uint64_t> uids = defense_set.get_equipped_uids();
    uint64_t weapon_uid = player_inventory.get_equipped_weapon_uid();
    if (weapon_uid != 0) {
        uids.push_back(weapon_uid);
    }
    return uids;
}

std::vector<std::string> Player::get_equipped_type_ids() const {
    // Mismo orden que get_equipped_uids(): defensas (armadura, casco, escudo) y
    // luego el arma, para que ambas listas queden índice a índice paralelas.
    std::vector<std::string> ids = defense_set.get_equipped_type_ids();
    std::string weapon_id = player_inventory.get_equipped_weapon_type_id();
    if (!weapon_id.empty()) {
        ids.push_back(weapon_id);
    }
    return ids;
}

int Player::calculate_defense() {
    return defense_set.calculate_defense();
}
 
bool Player::check_level_up() {
    if (level.try_level_up(experience)) {
        lives = max_life();
        mana  = max_mana();
        return true;
    }
    return false;
}

uint32_t Player::max_xp() const {
    return level.max_xp();
}

const std::string& Player::get_race_name() const {
    return player_race.get_name();
}

RaceType Player::get_race_id() const {
    return player_race.get_race_id();
}

ClassType Player::get_class_id() const {
    return player_class.get_class_id();
}

void Player::restore(uint32_t gold, uint32_t lives, uint32_t mana,
                     uint32_t experience, int level) {
    this->gold = gold;
    this->lives = lives;
    this->mana = mana;
    this->experience = experience;
    this->level = Level(level);
}

uint32_t Player::gold_drop() {
    return level.calculate_gold_drop(gold);
}
