#ifndef PLAYER_SERIALIZER_H_
#define PLAYER_SERIALIZER_H_

#include <string>

#include "../../common/constants/protocol_constants.h"
#include "../Game/item/item_catalog.h"
#include "../Game/player/player.h"
#include "player_record.h"

// Convierte entre el dominio del juego (Player) y los bytes persistidos
// (PlayerRecord). Es la capa que traduce dominio <-> formato de disco: vuelca
// los getters al record y reconstruye un Player desde un record (incluyendo el
// inventario via el catalogo de items).
class PlayerSerializer {
public:
    PlayerSerializer() = default;

    // Vuelca el estado de un Player a un PlayerRecord de tamaño fijo. La password
    // no la conoce el Player: se pasa aparte (viene del register o del record
    // previo).
    PlayerRecord to_record(const Player& player, Zone zone,
                           const std::string& password) const;

    // Reconstruye un Player a partir de un record (raza/clase/nombre via
    // constructor, estado mutable via restore(), inventario via el catalogo).
    Player from_record(const PlayerRecord& rec) const;

private:
    // Helpers de buffers de tamaño fijo (null-padded, truncan si no entran).
    void copy_to_fixed(char* dst, size_t n, const std::string& src) const;
    std::string read_fixed(const char* src, size_t n) const;

    // Setean raza/clase del dominio segun su enum del dominio.
    void set_race(PlayerRace& race, RaceType id) const;
    void set_class(PlayerClass& klass, ClassType id) const;

    ItemCatalog catalog;  // reconstruye los items del inventario por id
};

#endif  // PLAYER_SERIALIZER_H_
