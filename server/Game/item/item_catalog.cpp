#include "item_catalog.h"

#include "arma.h"
#include "armadura.h"
#include "baculo.h"
#include "casco.h"
#include "escudo.h"

// Tabla id -> constructor. Cada entrada conoce el subtipo y los stats exactos del
// item. Agregar aqui cualquier item nuevo que pueda terminar en un inventario
// (para que persista y se reconstruya correctamente).
//
// Los valores replican los que estaban hardcodeados en spawn_player /
// NPCseller::init_store (unica fuente de verdad ahora).
ItemCatalog::ItemCatalog() {
    // --- Armas (Arma: distancia_min, daño_min, daño_max) ---
    catalog["espada"]   = [] { return std::make_unique<Arma>("espada",   "Espada",   100, 1, 2, 5); };
    catalog["hacha"]    = [] { return std::make_unique<Arma>("hacha",    "Hacha",    150, 1, 4, 5); };
    catalog["martillo"] = [] { return std::make_unique<Arma>("martillo", "Martillo", 120, 1, 1, 9); };

    // --- Defensa (Escudo/Armadura/Casco: defensa_min, defensa_max) ---
    catalog["escudo"]   = [] { return std::make_unique<Escudo>("escudo", "Escudo de tortuga", 80, 1, 2); };

    // --- Magia (Baculo: tipo, costo_mana, distancia_min, valor_min, valor_max) ---
    catalog["vara_fresno"]      = [] { return std::make_unique<Baculo>("vara_fresno",      "Vara de fresno",   200, SpellKind::DANIO,   5, 8,  2,  4); };
    catalog["baculo_nudoso"]    = [] { return std::make_unique<Baculo>("baculo_nudoso",    "Báculo nudoso",    400, SpellKind::DANIO,  15, 8,  4,  8); };
    catalog["baculo_engarzado"] = [] { return std::make_unique<Baculo>("baculo_engarzado", "Báculo engarzado", 800, SpellKind::DANIO,  30, 8,  8, 20); };
    catalog["flauta_elfica"]    = [] { return std::make_unique<Baculo>("flauta_elfica",    "Flauta élfica",    600, SpellKind::CURAR, 100, 0, 30, 60); };
}

std::unique_ptr<Item> ItemCatalog::make_item(const std::string& id) const {
    auto it = catalog.find(id);
    if (it == catalog.end()) return nullptr;
    return it->second();
}

bool ItemCatalog::exists(const std::string& id) const {
    return catalog.find(id) != catalog.end();
}
