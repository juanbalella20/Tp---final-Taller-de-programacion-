#include "item_catalog.h"

#include <cstdlib>

#include "arma.h"
#include "armadura.h"
#include "baculo.h"
#include "casco.h"
#include "escudo.h"
#include "potion.h"
#include "../../../common/constants/game_config.h"

// Tabla id -> constructor. Cada entrada conoce el subtipo y los stats exactos del
// item. Agregar aqui cualquier item nuevo que pueda terminar en un inventario o
// caer al piso (para que persista y se reconstruya correctamente). Los stats se
// leen de GameConfig (config.toml) para no duplicar la fuente de verdad.
ItemCatalog::ItemCatalog() {
    const auto& cfg = GameConfig::instance();

    // --- Armas (Arma: distancia_min, daño_min, daño_max) ---
    const auto& es = cfg.espada;
    catalog["espada"]   = [es]   { return std::make_unique<Arma>("espada",   es.name,   es.price,   es.distance,   es.damage_min,   es.damage_max); };
    const auto& ha = cfg.hacha;
    catalog["hacha"]    = [ha]   { return std::make_unique<Arma>("hacha",    ha.name,   ha.price,   ha.distance,   ha.damage_min,   ha.damage_max); };
    const auto& ma = cfg.martillo;
    catalog["martillo"] = [ma]   { return std::make_unique<Arma>("martillo", ma.name,   ma.price,   ma.distance,   ma.damage_min,   ma.damage_max); };

    // --- Magia (Baculo: tipo, costo_mana, distancia_min, valor_min, valor_max) ---
    const auto& vf = cfg.vara_fresno;
    catalog["vara_fresno"]      = [vf] { return std::make_unique<Baculo>("vara_fresno",      vf.name, vf.price, SpellKind::DANIO, vf.mana_cost, vf.distance, vf.damage_min, vf.damage_max); };
    const auto& bn = cfg.baculo_nudoso;
    catalog["baculo_nudoso"]    = [bn] { return std::make_unique<Baculo>("baculo_nudoso",    bn.name, bn.price, SpellKind::DANIO, bn.mana_cost, bn.distance, bn.damage_min, bn.damage_max); };
    const auto& be = cfg.baculo_engarzado;
    catalog["baculo_engarzado"] = [be] { return std::make_unique<Baculo>("baculo_engarzado", be.name, be.price, SpellKind::DANIO, be.mana_cost, be.distance, be.damage_min, be.damage_max); };
    const auto& fe = cfg.flauta_elfica;
    catalog["flauta_elfica"]    = [fe] { return std::make_unique<Baculo>("flauta_elfica",    fe.name, fe.price, SpellKind::CURAR, fe.mana_cost, fe.distance, fe.damage_min, fe.damage_max); };

    // --- Armaduras (Armadura: defensa_min, defensa_max) ---
    const auto& ac = cfg.armadura_cuero;
    catalog["armadura_cuero"]  = [ac] { return std::make_unique<Armadura>("armadura_cuero",  ac.name, ac.price, ac.defense_min, ac.defense_max); };
    const auto& ap = cfg.armadura_placas;
    catalog["armadura_placas"] = [ap] { return std::make_unique<Armadura>("armadura_placas", ap.name, ap.price, ap.defense_min, ap.defense_max); };
    const auto& ta = cfg.tunica_azul;
    catalog["tunica_azul"]     = [ta] { return std::make_unique<Armadura>("tunica_azul",     ta.name, ta.price, ta.defense_min, ta.defense_max); };

    // --- Cascos (Casco: defensa_min, defensa_max) ---
    const auto& ca = cfg.capucha;
    catalog["capucha"]         = [ca] { return std::make_unique<Casco>("capucha",         ca.name, ca.price, ca.defense_min, ca.defense_max); };
    const auto& ch = cfg.casco_hierro;
    catalog["casco_hierro"]    = [ch] { return std::make_unique<Casco>("casco_hierro",    ch.name, ch.price, ch.defense_min, ch.defense_max); };
    const auto& sm = cfg.sombrero_magico;
    catalog["sombrero_magico"] = [sm] { return std::make_unique<Casco>("sombrero_magico", sm.name, sm.price, sm.defense_min, sm.defense_max); };

    // --- Escudos (Escudo: defensa_min, defensa_max) ---
    const auto& et = cfg.escudo_tortuga;
    catalog["escudo_tortuga"]  = [et] { return std::make_unique<Escudo>("escudo_tortuga", et.name, et.price, et.defense_min, et.defense_max); };
    // Alias historico "escudo" (lo usan spawn_player e inventarios persistidos).
    catalog["escudo"]          = [et] { return std::make_unique<Escudo>("escudo",         et.name, et.price, et.defense_min, et.defense_max); };
    const auto& eh = cfg.escudo_hierro;
    catalog["escudo_hierro"]   = [eh] { return std::make_unique<Escudo>("escudo_hierro",  eh.name, eh.price, eh.defense_min, eh.defense_max); };

    // --- Pociones (Potion: cura_vida, cura_mana) ---
    const auto& pv = cfg.pocion_vida;
    catalog["pocion_vida"]     = [pv] { return std::make_unique<Potion>("pocion_vida", pv.name, pv.price, pv.heal_life, pv.heal_mana); };
    const auto& pm = cfg.pocion_mana;
    catalog["pocion_mana"]     = [pm] { return std::make_unique<Potion>("pocion_mana", pm.name, pm.price, pm.heal_life, pm.heal_mana); };

    // Subconjunto de pociones para el drop "poción de vida o maná al azar".
    potion_ids = {"pocion_vida", "pocion_mana"};

    // Objetos elegibles para "cualquier otro objeto al azar": armas, magia,
    // armaduras, cascos y escudos (sin pociones ni el alias "escudo").
    droppable_item_ids = {
        "espada", "hacha", "martillo",
        "vara_fresno", "baculo_nudoso", "baculo_engarzado", "flauta_elfica",
        "armadura_cuero", "armadura_placas", "tunica_azul",
        "capucha", "casco_hierro", "sombrero_magico",
        "escudo_tortuga", "escudo_hierro",
    };
}

std::unique_ptr<Item> ItemCatalog::make_item(const std::string& id) const {
    auto it = catalog.find(id);
    if (it == catalog.end()) return nullptr;
    return it->second();
}

bool ItemCatalog::exists(const std::string& id) const {
    return catalog.find(id) != catalog.end();
}

std::unique_ptr<Item> ItemCatalog::make_random_potion() const {
    if (potion_ids.empty()) return nullptr;
    const std::string& id = potion_ids[std::rand() % potion_ids.size()];
    return make_item(id);
}

std::unique_ptr<Item> ItemCatalog::make_random_item() const {
    if (droppable_item_ids.empty()) return nullptr;
    const std::string& id = droppable_item_ids[std::rand() % droppable_item_ids.size()];
    return make_item(id);
}

std::unique_ptr<Item> ItemCatalog::make_random_item(
        const std::vector<std::string>& pool) const {
    // Filtramos a los ids que el catalogo conoce; un id mal escrito en el config
    // no debe romper el spawn de la zona, solo se descarta.
    std::vector<const std::string*> valid;
    for (const std::string& id : pool) {
        if (exists(id)) valid.push_back(&id);
    }
    if (valid.empty()) return make_random_item();  // pool global
    return make_item(*valid[std::rand() % valid.size()]);
}
