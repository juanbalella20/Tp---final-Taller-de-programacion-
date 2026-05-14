#ifndef PROTOCOL_CONSTANTS_H
#define PROTOCOL_CONSTANTS_H

#include <cstdint>

enum MessageType : uint8_t {
    MSG_REGISTER     = 0x01,  // Crear personaje nuevo
    MSG_LOGIN        = 0x02,  // Conectarse con personaje existente
    MSG_MOVE         = 0x03,  // Mover personaje
    MSG_ATTACK       = 0x04,  // Atacar jugador o NPC
    MSG_TAKE        = 0x05,  // Recoger item del suelo
    MSG_THROW        = 0x06,  // Tirar item del inventario
    MSG_EQUIP        = 0x07,  // Equipar item del inventario
    MSG_MEDITATE      = 0x08,  // Entrar/salir estado meditacion
    MSG_RESURRECT    = 0x09,  // Resucitar (fantasma)
    MSG_CURE        = 0x0A,  // Pedir curacion al sacerdote
    MSG_LIST       = 0x0B,  // Listar items de comerciante/banquero
    MSG_BUY        = 0x0C,  // Comprar item a sacerdote/comerciante
    MSG_SELL       = 0x0D,  // Vender item a comerciante
    MSG_DEPOSIT    = 0x0E,  // Depositar item en banco
    MSG_RETIRE      = 0x0F,  // Retirar item del banco
    MSG_DEP_GOLD      = 0x10,  // Depositar oro en banco
    MSG_RET_GOLD      = 0x11,  // Retirar oro del banco
    MSG_PRIVATE      = 0x12,  // Mensaje privado a otro jugador
    MSG_SELECT       = 0x13,  // Seleccionar entidad (click)
    MSG_FOUND_CLAN  = 0x14,  // Fundar un clan
    MSG_JOIN_CLAN  = 0x15,  // Pedir unirse a un clan
    MSG_REV_CLAN     = 0x16,  // Revisar pedidos del clan (fundador)
    MSG_CLAN_ACEP    = 0x17,  // Aceptar jugador al clan (fundador)
    MSG_CLAN_RECH    = 0x18,  // Rechazar jugador del clan (fundador)
    MSG_CLAN_BAN     = 0x19,  // Banear jugador del clan (fundador)
    MSG_CLAN_KICK    = 0x1A,  // Echar jugador del clan (fundador)
    MSG_LEFT_CLAN   = 0x1B,  // Dejar el clan
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

#endif  // PROTOCOL_CONSTANTS_H
