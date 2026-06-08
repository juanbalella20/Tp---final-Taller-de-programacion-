#ifndef CLAN_SERIALIZER_H_
#define CLAN_SERIALIZER_H_

#include <cstddef>
#include <string>

#include "../Game/player/clan.h"
#include "player_record.h"

// Traduce entre el dominio (Clan) y el POD persistido (ClanRecord). Espejo de
// PlayerSerializer: el record guarda founder/nombre/review + las listas de
// miembros y baneados (nombres null-padded). Las listas mas largas que el cupo
// fijo se truncan (limitacion documentada, igual que el inventario del player).
class ClanSerializer {
public:
    ClanRecord to_record(const Clan& clan) const;
    Clan from_record(const ClanRecord& rec) const;

private:
    // Copia src a un buffer fijo de n bytes, null-padded y siempre terminado.
    void copy_to_fixed(char* dst, size_t n, const std::string& src) const;
    // Lee un buffer fijo de n bytes a std::string (corta en el primer '\0').
    std::string read_fixed(const char* src, size_t n) const;
};

#endif  // CLAN_SERIALIZER_H_
