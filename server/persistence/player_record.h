#ifndef PLAYER_RECORD_H_
#define PLAYER_RECORD_H_

#include <cstdint>

#include "../../common/constants/game_constants.h"

// Persistencia binaria de jugadores (consigna): structs de tamaño FIJO y
// constante que se vuelcan crudos a players.dat. El record i vive en el offset
// i * sizeof(PlayerRecord), por lo que un seek + read directo lo recupera.
//
// Limitaciones asumidas (documentadas):
//  - Formato dependiente de arquitectura/endianness (POD packed, sin std::string).
//    Valido porque el server corre en una sola maquina.
//  - Nombres/ids/passwords mas largos que el buffer fijo se truncan.

#pragma pack(push, 1)

// Un slot del inventario persistido. Guarda SOLO el id del item; los stats se
// reconstruyen via item_catalog (unica fuente de verdad), no se persisten.
struct ItemRecord {
    char    id[PERSIST_ITEM_ID_MAX];  // null-padded
    uint8_t occupied;                 // 0 = slot vacio, 1 = ocupado
    uint8_t equipped;                 // 0/1: equipado en su slot
};

// Datos completos de un jugador. Tamaño fijo y constante (RECORD_SIZE).
struct PlayerRecord {
    char     name[PERSIST_NAME_MAX];          // PK, null-padded
    char     password[PERSIST_PASSWORD_MAX];  // texto plano, null-padded
    uint8_t  race;        // enum Race  (inmutable)
    uint8_t  klass;       // enum Class (inmutable)
    uint8_t  zone;        // enum Zone
    uint8_t  is_ghost;    // 0/1
    uint8_t  meditating;  // 0/1
    int32_t  level;
    uint32_t gold;
    uint32_t lives;       // HP actual
    uint32_t mana;
    uint32_t experience;
    int32_t  coord_x;
    int32_t  coord_y;
    int32_t  id_clan;     // -1 si ninguno
    uint8_t  inv_count;   // cantidad de slots ocupados en items[]
    ItemRecord items[PERSIST_INV_SLOTS];
};

// Una entrada del indice persistido (index.dat): nombre -> offset en players.dat.
struct IndexEntry {
    char     name[PERSIST_NAME_MAX];  // null-padded
    uint64_t offset;                  // byte offset del record en players.dat
};

#pragma pack(pop)

#endif  // PLAYER_RECORD_H_
