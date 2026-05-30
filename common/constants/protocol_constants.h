#ifndef PROTOCOL_CONSTANTS_H
#define PROTOCOL_CONSTANTS_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include "game_constants.h"

const uint16_t LEN_HEADER = 3;  // 1 byte para tipo de mensaje + 2 bytes para largo del payload
const uint16_t LEN_NAME_SIZE_FIELD = 1;  // 1 byte para largo del nombre
const uint16_t LEN_RACE = 1;  // 1 byte para la raza
const uint16_t LEN_CLASS = 1;  // 1 byte para la clase
const uint16_t LEN_ENTITY = 1;  // 1 byte para el tipo de entidad (player/npc)
const uint16_t LEN_DIRECTION = 1;
const uint16_t LEN_COORD = 2;  // 2 bytes para cada coordenada (x e y)
const uint16_t LEN_ZONE = 1;  // 1 byte para el valor de zona
// MSG_ZONE_CHANGE: [zona:1B][x:2B BE][y:2B BE] -> zona destino + posicion de spawn
const uint16_t LEN_ZONE_CHANGE_PAYLOAD = LEN_ZONE + 2 * LEN_COORD;
const uint16_t LEN_ITEM_ID = 1;  // 1 byte para largo del id del item
const uint16_t LEN_NPC_COUNT = 2;  // 2 bytes para la cantidad de NPCs en el snapshot
const uint16_t LEN_NPC_TYPE_SIZE = 1;  // 1 byte para largo del tipo del NPC
const uint16_t LEN_NPC_NAME_SIZE = 1;  // 1 byte para largo del nombre del NPC
const uint16_t LEN_ITEM_COUNT = 2;  // 2 bytes para la cantidad de items en el snapshot
const uint16_t LEN_ITEM_TYPE_SIZE = 1;  // 1 byte para largo del tipo del item

enum MessageType : uint8_t {
    MSG_REGISTER     = 0x01,  // Crear personaje nuevo            [cliente OK] [servidor OK]
    MSG_LOGIN        = 0x02,  // Conectarse con personaje         [x cliente]  [x servidor]
    MSG_MOVE         = 0x03,  // Mover personaje                  [cliente OK] [servidor OK]
    MSG_ATTACK       = 0x04,  // Atacar jugador o NPC             [cliente OK] [servidor OK]
    MSG_TAKE         = 0x05,  // Recoger item del suelo           [x cliente]  [x servidor]
    MSG_THROW        = 0x06,  // Tirar item del inventario        [cliente OK] [servidor OK]
    MSG_EQUIP        = 0x07,  // Equipar item del inventario      [cliente OK] [servidor OK]
    MSG_MEDITATE     = 0x08,  // Entrar/salir estado meditacion   [cliente OK] [servidor OK]
    MSG_RESURRECT    = 0x09,  // Resucitar (fantasma)             [cliente OK] [servidor OK]
    MSG_CURE         = 0x0A,  // Pedir curacion al sacerdote      [cliente OK] [servidor OK]
    MSG_LIST         = 0x0B,  // Listar items comerciante/banquero[cliente OK] [servidor OK]
    MSG_BUY          = 0x0C,  // Comprar item                     [cliente OK] [servidor OK]
    MSG_SELL         = 0x0D,  // Vender item a comerciante        [cliente OK] [servidor OK]
    MSG_DEPOSIT      = 0x0E,  // Depositar item en banco          [cliente OK] [servidor OK]
    MSG_RETIRE       = 0x0F,  // Retirar item del banco           [cliente OK] [servidor OK]
    MSG_DEP_GOLD     = 0x10,  // Depositar oro en banco           [cliente OK] [servidor OK]
    MSG_RET_GOLD     = 0x11,  // Retirar oro del banco            [cliente OK] [servidor OK]
    MSG_PRIVATE      = 0x12,  // Mensaje privado a otro jugador   [x cliente]  [x servidor]
    MSG_SELECT       = 0x13,  // Seleccionar entidad (click)      [cliente OK] [servidor OK]
    MSG_FOUND_CLAN   = 0x14,  // Fundar un clan                   [cliente OK] [x servidor]
    MSG_JOIN_CLAN    = 0x15,  // Pedir unirse a un clan           [cliente OK] [x servidor]
    MSG_REV_CLAN     = 0x16,  // Revisar pedidos del clan         [cliente OK] [servidor OK]
    MSG_CLAN_ACEP    = 0x17,  // Aceptar jugador al clan          [cliente OK] [x servidor]
    MSG_CLAN_RECH    = 0x18,  // Rechazar jugador del clan        [cliente OK] [x servidor]
    MSG_CLAN_BAN     = 0x19,  // Banear jugador del clan          [cliente OK] [x servidor]
    MSG_CLAN_KICK    = 0x1A,  // Echar jugador del clan           [cliente OK] [x servidor]
    MSG_LEFT_CLAN    = 0x1B,  // Dejar el clan                    [cliente OK] [servidor OK]
    MSG_SEND_MAP     = 0x1C,  // Enviar mapa al cliente      
    MSG_INVENTORY    = 0x1D,  // Enviar inventario al cliente             
    MSG_CHAT         = 0x1E,  // Mensajes generales
    MSG_CHEAT_KILL   = 0x1F,  // Cheat para morir automáticamente
    MSG_CHEAT_INF_HP = 0x20,  // Cheat para vida infinita
    MSG_CHEAT_INF_MANA = 0x21, // Cheat para maná infinito
    MSG_GOLD = 0x22,          // Actualizar oro del jugador
    MSG_HP = 0x23,            // Actualizar hp del jugador
    MSG_XP = 0x24,            // Actualizar xp del jugador
    MSG_MANA = 0x25,            // Actualizar mana del jugador
    MSG_NPCS_SNAPSHOT  = 0x26, // Snapshot de NPCs vivos: vector<NpcInfo>
    MSG_ITEMS_SNAPSHOT = 0x27, // Snapshot de items en el piso: vector<ItemFloorInfo>
    MSG_TELEPORT       = 0x29, // Cliente->server: pedido de /tp (sin payload)
    MSG_ZONE_CHANGE = 0x28, // Server -> cliente: zona a cargar
};

enum Direction : uint8_t {
    DIR_NORTH = 0x00,  // Mover hacia arriba
    DIR_SOUTH   = 0x01,  // Mover hacia abajo
    DIR_EAST  = 0x02,  // Mover hacia la derecha
    DIR_WEST = 0x03,  // Mover hacia la izquierda
};

enum EntityType : uint8_t {
    ENTITY_PLAYER = 0x00,  // La entidad es un jugador
    ENTITY_NPC    = 0x01,  // La entidad es un NPC
};

enum Race : uint8_t {
    RACE_HUMAN  = 0x00,  // Humano
    RACE_ELF    = 0x01,  // Elfo
    RACE_DWARF  = 0x02,  // Enano
    RACE_GNOME  = 0x03,  // Gnomo
};

enum Class : uint8_t {
    MAGE    = 0x00,  
    CLERIC  = 0x01,  
    PALADIN = 0x02, 
    WARRIOR = 0x03,  
};
enum Sector : uint8_t {
    SECTOR_FOREST = 0x00,
    SECTOR_TOWN   = 0x01,
    SECTOR_CITY   = 0x02,
    SECTOR_DESERT = 0x03,
};


enum ELEMENT_TYPE : uint8_t {
    ELEMENT_PLAYER = 0x00,
    ELEMENT_NPC    = 0x01,
    ELEMENT_OBJECT = 0x02,
    ELEMENT_BUILDING = 0x03,
    ELEMENT_GOLD = 0x04,
    ELEMENT_EMPTY = 0x05,
};

enum Zone : uint8_t {
    ZONE_DESERT = 0x00,
    ZONE_CITY = 0x01,
    ZONE_FOREST = 0x02,
    ZONE_TOWN = 0x03,
};

// Nombre de zona (usado en el TOML para dest_zone y en el label del cliente).
const std::unordered_map<std::string, Zone> ZONE_NAME_MAP = {
    {"desert", ZONE_DESERT},
    {"city",   ZONE_CITY},
    {"forest", ZONE_FOREST},
    {"town",   ZONE_TOWN},
};

const std::unordered_map<uint8_t, std::string> ZONE_NAME_MAP_INV = {
    {ZONE_DESERT, "desert"},
    {ZONE_CITY,   "city"},
    {ZONE_FOREST, "forest"},
    {ZONE_TOWN,   "town"},
};

// Solo buildings/empty viven en la matriz (terreno estatico).
// Actores (players/npcs) y objetos del piso (objects/gold) viajan en
// snapshots dedicados: MSG_NPCS_SNAPSHOT y MSG_ITEMS_SNAPSHOT.
const std::unordered_map<elements, ELEMENT_TYPE> ELEMENT_TYPE_MAP = {
    {elements::buildings, ELEMENT_BUILDING},
    {elements::empty, ELEMENT_EMPTY},
    {elements::npcs, ELEMENT_NPC},
};

const std::unordered_map<std::string, Race> RACE_MAP = {
    {"human",  RACE_HUMAN},
    {"elf",    RACE_ELF},
    {"dwarf",  RACE_DWARF},
    {"gnome",  RACE_GNOME},
};

const std::unordered_map<uint8_t, std::string> RACE_MAP_INV = {
    {RACE_HUMAN,  "human"},
    {RACE_ELF,    "elf"},
    {RACE_DWARF,  "dwarf"},
    {RACE_GNOME,  "gnome"},
};

const std::unordered_map<std::string, Class> CLASS_MAP = {
    {"mage",    MAGE},
    {"cleric",  CLERIC},
    {"paladin", PALADIN},
    {"warrior", WARRIOR},
};

const std::unordered_map<uint8_t, std::string> CLASS_MAP_INV = {
    {MAGE,    "mage"},
    {CLERIC,  "cleric"},
    {PALADIN, "paladin"},
    {WARRIOR, "warrior"},
};

#endif  // PROTOCOL_CONSTANTS_H
