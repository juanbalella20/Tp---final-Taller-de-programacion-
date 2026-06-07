#ifndef ITEM_CATALOG_H_
#define ITEM_CATALOG_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

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

private:
    using Factory = std::function<std::unique_ptr<Item>()>;
    std::unordered_map<std::string, Factory> catalog;
};

#endif  // ITEM_CATALOG_H_
