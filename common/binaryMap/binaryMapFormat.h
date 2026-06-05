#ifndef BINARY_MAP_FORMAT_H
#define BINARY_MAP_FORMAT_H

#include <cstdint>

/*
 * Formato binario de mapas del editor (.bin).
 *
 * Objetivos de diseno:
 *  - Espejar el modelo logico del TOML (tile_size, width/height, tilesets con
 *    firstgid, capas de IDs, spawns, teleports) para que el server pueda
 *    consumirlo via BinaryMapLoader con cambio minimo.
 *  - Ser EXTENSIBLE a NPCs/items/portales sin romper lectores viejos: se usa un
 *    header versionado + una tabla de SECCIONES TIPADAS (chunks). Un lector que
 *    no conoce un section_type salta byte_length bytes y sigue.
 *  - Endianness FIJA big-endian (network byte order), con marcador en el header.
 *    Se elige big-endian para alinear con el protocolo de red del proyecto
 *    (htons/htonl y ntohs/ntohl). Nunca se escribe un struct completo: eso evita
 *    padding/alineacion no portable. Como se convierte a network byte order, el
 *    archivo es identico en cualquier arquitectura.
 *
 * --- Layout ---------------------------------------------------------------
 *
 * Todos los enteros multibyte van en BIG-ENDIAN (byte mas significativo primero).
 *
 * Header (16 bytes):
 *   offset 0  : char[8]  magic              = "AOMAPBIN" (sin NUL)
 *   offset 8  : uint16   format_version     = 1
 *   offset 10 : uint16   endianness_marker  = 0x0102
 *               (se escribe big-endian como bytes 01 02; si un lector lo
 *                interpreta como 0x0201, el archivo es del otro endianness)
 *   offset 12 : uint32   section_count
 *
 * Luego section_count secciones, cada una:
 *   uint16  section_type
 *   uint32  section_byte_length   (bytes del payload que sigue; permite skip)
 *   byte[section_byte_length] payload
 *
 * Convenciones de payload:
 *   - Strings: uint16 len + len bytes UTF-8 (sin terminador NUL).
 *   - Tile IDs (gid): uint16. 0 = celda vacia.
 *     (buildings tiene 256 tiles y los firstgid se suman en rangos -> uint8 se
 *      desborda; uint16 cubre 65535, holgado.)
 *   - tile_size / width / height: uint32.
 *   - Coordenadas (x, y): int32.
 *
 * Payload de cada seccion conocida:
 *
 *   META (0x0001) -- obligatoria, una sola:
 *     uint32 tile_size
 *     uint32 width
 *     uint32 height
 *
 *   TILESETS (0x0002):
 *     uint32 tileset_count
 *     repeat tileset_count:
 *       string name
 *       string file_path        (RELATIVO al .bin: "sand.png", "tiles/x.png")
 *       uint32 columns
 *       uint32 tile_count
 *       uint32 firstgid
 *       uint8  collidable       (0/1)
 *
 *   LAYERS (0x0003):
 *     uint32 layer_count
 *     repeat layer_count:
 *       string name
 *       repeat height: repeat width: uint16 gid     (row-major, dims de META)
 *
 *   SPAWNS (0x0004):
 *     uint32 spawn_count
 *     repeat spawn_count:
 *       string name
 *       int32  x
 *       int32  y
 *
 *   TELEPORTS (0x0005):
 *     uint32 teleport_count
 *     repeat teleport_count:
 *       int32  x
 *       int32  y
 *       string dest_zone
 *
 * Sumar una seccion futura (NPC/items/portales) = nuevo section_type 0x0010+
 * y subir section_count. NO se toca nada de lo anterior.
 */

// Constantes del formato binario, agrupadas como miembros static de una clase
// (sin instanciar). Acceso: BinaryMapFormat::MAGIC, BinaryMapFormat::Section::META, etc.
class BinaryMapFormat {
public:
    BinaryMapFormat() = delete;  // solo constantes; no se instancia

    // Magic de 8 bytes ASCII (sin terminador NUL en el archivo).
    static constexpr char     MAGIC[8]          = {'A', 'O', 'M', 'A', 'P', 'B', 'I', 'N'};
    static constexpr uint16_t FORMAT_VERSION    = 1;
    // Marcador de endianness. Se escribe/lee en big-endian (bytes 01 02). Un
    // lector que lo interprete como 0x0201 sabe que el archivo es del endianness
    // opuesto.
    static constexpr uint16_t ENDIANNESS_MARKER = 0x0102;
    static constexpr uint32_t HEADER_SIZE       = 16;

    // Tipos de seccion. Los reservados 0x0010+ permiten extender sin romper
    // lectores viejos (un lector salta las secciones que no conoce).
    enum class Section : uint16_t {
        META      = 0x0001,
        TILESETS  = 0x0002,
        LAYERS    = 0x0003,
        SPAWNS    = 0x0004,
        TELEPORTS = 0x0005,

        // --- Reservados para futuras iteraciones (sin tocar la arquitectura) ---
        NPC_FRIENDLY = 0x0010,
        NPC_HOSTILE  = 0x0011,
        ITEMS        = 0x0012,
        PORTALS      = 0x0013,
    };
};

#endif  // BINARY_MAP_FORMAT_H
