#ifndef ITEM_CATALOG_H_
#define ITEM_CATALOG_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "item.h"

// Catalogo / fabrica de items por id. Unica fuente de verdad de los stats de
// cada item del juego: el inventario persistido guarda SOLO el id de cada item
// (PlayerRecord), y al cargar se reconstruye el item completo (subtipo + stats)
// con make_item(id).
//
// Reemplaza la creacion hardcodeada y dispersa de items (spawn_player,
// NPCseller) por un unico registro, y de paso arregla el bug del vendedor que
// reconstruia todo como Arma con stats inventados.
class ItemCatalog {
public:
    ItemCatalog();

    // Construye el item con ese id (subtipo + stats correctos), o nullptr si el
    // id no existe en el catalogo.
    std::unique_ptr<Item> make_item(const std::string& id) const;

    // ¿El id existe en el catalogo?
    bool exists(const std::string& id) const;

    // Construye una poción (vida o maná) elegida al azar. Para el drop de NPCs.
    std::unique_ptr<Item> make_random_potion() const;

    // Construye cualquier objeto del catálogo elegido al azar (incluye pociones).
    // Para el drop "cualquier otro objeto al azar" de los NPCs.
    std::unique_ptr<Item> make_random_item() const;

private:
    using Factory = std::function<std::unique_ptr<Item>()>;
    std::unordered_map<std::string, Factory> catalog;
    // ids de las pociones registradas (subconjunto de catalog), para el drop
    // "una poción de vida o maná al azar".
    std::vector<std::string> potion_ids;
    // ids elegibles para el drop "cualquier otro objeto al azar": todos los
    // objetos del catálogo salvo las pociones y los alias duplicados.
    std::vector<std::string> droppable_item_ids;
};

#endif  // ITEM_CATALOG_H_
