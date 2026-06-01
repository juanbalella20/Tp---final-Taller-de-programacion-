#include "binaryMapSaver.h"

#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <stdexcept>

#include "binaryMapFormat.h"

// ============================================================================
//
// Serializar a 'path' en BIG-ENDIAN usando network byte order:
//   1. Header: MAGIC, FORMAT_VERSION, ENDIANNESS_MARKER, section_count=3.
//   2. Seccion META      (tile_size, width, height).
//   3. Seccion TILESETS  (count + por tileset: name, file_path, columns,
//                         tile_count, firstgid, collidable).
//   4. Seccion LAYERS    (count + por capa: name + matriz height*width uint16).
// Cada seccion lleva su [type:uint16][byte_length:uint32] por delante.
// V1 NO escribe SPAWNS/TELEPORTS (secciones reservadas para el futuro).
// Lanzar std::runtime_error ante error de I/O.
//
// Sugerencia: helpers locales write_u16/write_u32/write_i32/write_string que
// conviertan a network byte order y armen cada payload en un buffer para conocer
// su byte_length antes de escribir el encabezado de la seccion.
// ============================================================================

// Buffer de bytes que crece. Acumulamos cada payload aca para conocer su
// byte_length ANTES de escribir el encabezado [type][byte_length] de la
// seccion (el formato lo necesita por delante para permitir skip).
using ByteBuffer = std::vector<uint8_t>;

static void append_bytes(ByteBuffer& buf, const void* data, size_t size) {
    const auto* bytes = static_cast<const uint8_t*>(data);
    buf.insert(buf.end(), bytes, bytes + size);
}

static void write_u16(ByteBuffer& buf, uint16_t v) {
    uint16_t network_value = htons(v);
    append_bytes(buf, &network_value, sizeof(network_value));
}

static void write_u32(ByteBuffer& buf, uint32_t v) {
    uint32_t network_value = htonl(v);
    append_bytes(buf, &network_value, sizeof(network_value));
}

// String: uint16 len + len bytes UTF-8 (sin terminador NUL).
static void write_string(ByteBuffer& buf, const std::string& s) {
    if (s.size() > 0xFFFF) {
        throw std::runtime_error("BinaryMapSaver: string demasiado largo (>65535)");
    }
    write_u16(buf, static_cast<uint16_t>(s.size()));
    buf.insert(buf.end(), s.begin(), s.end());
}

// Escribe una seccion completa al buffer de salida: [type][byte_length][payload].
static void write_section(ByteBuffer& out, BinaryMapFormat::Section type,
                          const ByteBuffer& payload) {
    write_u16(out, static_cast<uint16_t>(type));
    write_u32(out, static_cast<uint32_t>(payload.size()));
    out.insert(out.end(), payload.begin(), payload.end());
}

void BinaryMapSaver::save(const std::string& path,
                          int tile_size, int width, int height,
                          const std::vector<Tileset>& tilesets,
                          const std::vector<MapLayerData>& layers) {
    ByteBuffer out;

    // --- Header (16 bytes) ---------------------------------------------------
    // MAGIC (8 bytes, sin NUL) + version + marcador de endianness + section_count.
    out.insert(out.end(), std::begin(BinaryMapFormat::MAGIC),
               std::end(BinaryMapFormat::MAGIC));
    write_u16(out, BinaryMapFormat::FORMAT_VERSION);
    write_u16(out, BinaryMapFormat::ENDIANNESS_MARKER);
    write_u32(out, 3);  // section_count = META + TILESETS + LAYERS (V1)

    // --- Seccion META --------------------------------------------------------
    {
        ByteBuffer meta;
        write_u32(meta, static_cast<uint32_t>(tile_size));
        write_u32(meta, static_cast<uint32_t>(width));
        write_u32(meta, static_cast<uint32_t>(height));
        write_section(out, BinaryMapFormat::Section::META, meta);
    }

    // --- Seccion TILESETS ----------------------------------------------------
    {
        ByteBuffer ts_buf;
        write_u32(ts_buf, static_cast<uint32_t>(tilesets.size()));
        for (const Tileset& ts : tilesets) {
            write_string(ts_buf, ts.name);
            // file_path se guarda RELATIVO al .bin (el llamador debe pasarlo asi).
            write_string(ts_buf, ts.file_path);
            write_u32(ts_buf, static_cast<uint32_t>(ts.columns));
            write_u32(ts_buf, static_cast<uint32_t>(ts.tile_count));
            write_u32(ts_buf, static_cast<uint32_t>(ts.firstgid));
            ts_buf.push_back(ts.collidable ? 1 : 0);  // uint8 0/1
        }
        write_section(out, BinaryMapFormat::Section::TILESETS, ts_buf);
    }

    // --- Seccion LAYERS ------------------------------------------------------
    // count + por capa: name + matriz height*width row-major de uint16 gid.
    {
        ByteBuffer ly_buf;
        write_u32(ly_buf, static_cast<uint32_t>(layers.size()));
        for (const MapLayerData& layer : layers) {
            write_string(ly_buf, layer.name);
            // Precondicion del contrato: layer.data es [height][width]. Validamos
            // las dimensiones para no emitir un .bin truncado/corrupto silenciosamente.
            if (static_cast<int>(layer.data.size()) != height) {
                throw std::runtime_error(
                    "BinaryMapSaver: capa '" + layer.name + "' tiene " +
                    std::to_string(layer.data.size()) + " filas, se esperaban " +
                    std::to_string(height));
            }
            for (const std::vector<int>& row : layer.data) {
                if (static_cast<int>(row.size()) != width) {
                    throw std::runtime_error(
                        "BinaryMapSaver: capa '" + layer.name +
                        "' tiene una fila de " + std::to_string(row.size()) +
                        " columnas, se esperaban " + std::to_string(width));
                }
                for (int gid : row) {
                    // gid 0 = celda vacia. Cabe en uint16 (buildings llega a 256+).
                    if (gid < 0 || gid > 0xFFFF) {
                        throw std::runtime_error(
                            "BinaryMapSaver: gid fuera de rango uint16 en capa '" +
                            layer.name + "'");
                    }
                    write_u16(ly_buf, static_cast<uint16_t>(gid));
                }
            }
        }
        write_section(out, BinaryMapFormat::Section::LAYERS, ly_buf);
    }

    // --- Volcado a disco -----------------------------------------------------
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        throw std::runtime_error("BinaryMapSaver: no se pudo abrir para escribir: " + path);
    }
    file.write(reinterpret_cast<const char*>(out.data()),
               static_cast<std::streamsize>(out.size()));
    if (!file) {
        throw std::runtime_error("BinaryMapSaver: error de escritura en: " + path);
    }
}
