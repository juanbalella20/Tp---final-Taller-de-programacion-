#include "player_serializer.h"

#include <cstring>

void PlayerSerializer::copy_to_fixed(char* dst, size_t n, const std::string& src) const {
    std::memset(dst, 0, n);
    std::strncpy(dst, src.c_str(), n - 1);
}

std::string PlayerSerializer::read_fixed(const char* src, size_t n) const {
    return std::string(src, ::strnlen(src, n));
}

void PlayerSerializer::set_race(PlayerRace& race, RaceType id) const {
    switch (id) {
        case RaceType::HUMAN: race.set_human(); break;
        case RaceType::ELF:   race.set_elf();   break;
        case RaceType::DWARF: race.set_dwarf(); break;
        case RaceType::GNOME: race.set_gnome(); break;
    }
}

void PlayerSerializer::set_class(PlayerClass& klass, ClassType id) const {
    switch (id) {
        case ClassType::MAGE:    klass.set_wizard();  break;
        case ClassType::CLERIC:  klass.set_cleric();  break;
        case ClassType::PALADIN: klass.set_paladin(); break;
        case ClassType::WARRIOR: klass.set_warrior(); break;
    }
}

PlayerRecord PlayerSerializer::to_record(const Player& player, Zone zone,
                                         const std::string& password) const {
    PlayerRecord rec;
    std::memset(&rec, 0, sizeof(rec));

    copy_to_fixed(rec.name, PERSIST_NAME_MAX, player.get_name());
    copy_to_fixed(rec.password, PERSIST_PASSWORD_MAX, password);

    // RaceType/ClassType del dominio comparten valores con el protocolo: cast directo.
    rec.race  = static_cast<uint8_t>(player.get_race_id());
    rec.klass = static_cast<uint8_t>(player.get_class_id());
    rec.zone  = static_cast<uint8_t>(zone);

    rec.is_ghost   = player.is_ghost() ? 1 : 0;
    rec.meditating = 0;  // no se persiste meditando: al reconectar arranca vivo
    rec.level      = player.get_level();
    rec.gold       = player.get_gold();
    rec.lives      = player.get_lives();
    rec.mana       = player.get_mana();
    rec.experience = player.get_xp();
    rec.coord_x    = player.get_coord_x();
    rec.coord_y    = player.get_coord_y();
    rec.id_clan    = player.get_clan_id();

    // Inventario: un slot por item, guardando SOLO el id. Marca los equipados.
    std::vector<std::string> equipped = player.get_equipped_ids();
    auto is_equipped = [&equipped](const std::string& id) {
        for (const auto& e : equipped) {
            if (e == id) return true;
        }
        return false;
    };

    uint8_t count = 0;
    for (Item* item : player.get_all_items()) {
        if (count >= PERSIST_INV_SLOTS) break;
        ItemRecord& slot = rec.items[count];
        copy_to_fixed(slot.id, PERSIST_ITEM_ID_MAX, item->get_id());
        slot.occupied = 1;
        slot.equipped = is_equipped(item->get_id()) ? 1 : 0;
        ++count;
    }
    rec.inv_count = count;

    return rec;
}

Player PlayerSerializer::from_record(const PlayerRecord& rec) const {
    PlayerRace race;
    PlayerClass klass;
    set_race(race, static_cast<RaceType>(rec.race));
    set_class(klass, static_cast<ClassType>(rec.klass));

    std::string name = read_fixed(rec.name, PERSIST_NAME_MAX);
    Player player(name, race, klass);

    // Estado mutable que el constructor calcula / no tiene setter.
    player.restore(rec.gold, rec.lives, rec.mana, rec.experience, rec.level, rec.id_clan);
    player.update_position(rec.coord_x, rec.coord_y);
    if (rec.is_ghost) player.set_ghost();

    // Inventario: reconstruye cada item via catalogo y reequipa los marcados.
    for (uint8_t i = 0; i < rec.inv_count && i < PERSIST_INV_SLOTS; ++i) {
        const ItemRecord& slot = rec.items[i];
        if (!slot.occupied) continue;
        std::string id = read_fixed(slot.id, PERSIST_ITEM_ID_MAX);
        auto item = catalog.make_item(id);
        if (!item) continue;  // id desconocido (catalogo desactualizado): se ignora
        player.add_item(std::move(item));
        if (slot.equipped) player.equip_item(id);
    }

    return player;
}
