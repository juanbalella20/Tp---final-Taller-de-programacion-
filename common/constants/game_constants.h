#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

#include <cstdint>

// Identidad de raza/clase del DOMINIO del juego (capa comun, sin protocolo).
// El dominio (PlayerRace/PlayerClass) y la persistencia (PlayerRecord) usan
// estos enums; la capa de red traduce entre estos y su propio enum del
// protocolo. Los valores coinciden con los del protocolo para que la traduccion
// sea directa, pero el dominio NO depende de protocol_constants.h.
enum class RaceType : uint8_t {
    HUMAN = 0,
    ELF   = 1,
    DWARF = 2,
    GNOME = 3,
};

enum class ClassType : uint8_t {
    MAGE    = 0,
    CLERIC  = 1,
    PALADIN = 2,
    WARRIOR = 3,
};

struct positionCoord {
    int x;
    int y;
};

struct sectorPerimiter {
    positionCoord top_left;
    positionCoord top_right;
    positionCoord bottom_left;
    positionCoord bottom_right;
};

struct groundGold {
    positionCoord pos;
    int amount;
};

enum class elements {
    buildings,
    empty,
    npcs
};

#define HEIGHT 16
#define WIDTH 30
// Tamano de tile en pixeles. Lo usan el editor grafico, el formato .bin y el
// render. Fijo en V1 (mapas 30x16 con tiles de 64x64).
#define TILE_SIZE 64

// ---- Persistencia de jugadores ----
// Dos archivos binarios (consigna): players.dat (records de tamaño fijo) +
// index.dat (diccionario nombre -> offset). Paths relativos al cwd como el
// resto del proyecto; el directorio/archivos se crean al iniciar si no existen.
#define PERSIST_DATA_DIR "data/persistence/"
#define PERSIST_PLAYERS_FILE "players.dat"
#define PERSIST_INDEX_FILE "index.dat"

// Guardado periodico: cada cuantos ticks del game loop se persiste el estado de
// los jugadores online. Loop a 20 ticks/s => 600 ticks = 30 s.
#define PERSIST_INTERVAL_TICKS 600

// Tamaños fijos de los buffers del PlayerRecord (POD). Nombres/ids/passwords mas
// largos se truncan (limitacion documentada).
#define PERSIST_NAME_MAX 32
#define PERSIST_PASSWORD_MAX 32
#define PERSIST_ITEM_ID_MAX 24
#define PERSIST_INV_SLOTS 25  // debe coincidir con Inventory::MAX_SLOTS


#endif