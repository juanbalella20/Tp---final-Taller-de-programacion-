#include "binaryMapLoader.h"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <utility>

#include "binaryMapFormat.h"
#include "byteReader.h"

void BinaryMapLoader::load(const std::string& path) {
    // read_file: carga el .bin completo y valida que tenga al menos el header.
    std::vector<uint8_t> bytes = read_file(path);

    // reset_state: limpia datos anteriores por si se reutiliza esta instancia.
    reset_state();

    ByteReader reader(bytes.data(), bytes.size());

    // read_header: valida MAGIC, version y endianness; devuelve section_count.
    uint32_t section_count = read_header(reader);

    // parse_sections: recorre las secciones tipadas, parsea las conocidas y
    // saltea las desconocidas para mantener compatibilidad hacia adelante.
    std::string base_dir = std::filesystem::path(path).parent_path().string();
    parse_sections(reader, bytes, base_dir, section_count);

    // build_tile_index: arma el indice gid -> TileDef usado por find_tile/collision.
    build_tile_index();
}

std::vector<uint8_t> BinaryMapLoader::read_file(const std::string& path) const {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("BinaryMapLoader: no se pudo abrir: " + path);
    }
    std::streamsize file_size = file.tellg();
    if (file_size < static_cast<std::streamsize>(BinaryMapFormat::HEADER_SIZE)) {
        throw std::runtime_error("BinaryMapLoader: archivo mas chico que el header: " + path);
    }
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> bytes(static_cast<size_t>(file_size));
    if (!file.read(reinterpret_cast<char*>(bytes.data()), file_size)) {
        throw std::runtime_error("BinaryMapLoader: error de lectura en: " + path);
    }

    return bytes;
}

void BinaryMapLoader::reset_state() {
    tile_size = 0;
    width = 0;
    height = 0;
    tilesets.clear();
    layers.clear();
    tiles.clear();
    spawns.clear();
    teleports.clear();
}

uint32_t BinaryMapLoader::read_header(ByteReader& reader) const {
    reader.require(8);
    char magic[8];
    for (int i = 0; i < 8; ++i) magic[i] = static_cast<char>(reader.read_u8());
    if (std::memcmp(magic, BinaryMapFormat::MAGIC, 8) != 0) {
        throw std::runtime_error("BinaryMapLoader: magic invalido (no es un .bin AOMAPBIN)");
    }

    uint16_t version = reader.read_u16();
    if (version > BinaryMapFormat::FORMAT_VERSION) {
        throw std::runtime_error(
            "BinaryMapLoader: format_version " + std::to_string(version) +
            " mas nuevo que el soportado (" +
            std::to_string(BinaryMapFormat::FORMAT_VERSION) + ")");
    }

    uint16_t endian = reader.read_u16();
    if (endian != BinaryMapFormat::ENDIANNESS_MARKER) {
        // 0x0201 indicaria un archivo del endianness opuesto; cualquier otro
        // valor, corrupcion.
        throw std::runtime_error("BinaryMapLoader: marcador de endianness invalido");
    }

    return reader.read_u32();
}

void BinaryMapLoader::parse_sections(ByteReader& reader, const std::vector<uint8_t>& bytes,
                                     const std::string& base_dir,
                                     uint32_t section_count) {
    bool seen_meta = false;

    for (uint32_t s = 0; s < section_count; ++s) {
        uint16_t type = reader.read_u16();
        uint32_t byte_length = reader.read_u32();
        reader.require(byte_length);

        // Sub-lector acotado al payload de esta seccion: garantiza que el parseo
        // de una seccion no se pase a la siguiente.
        size_t section_start = reader.pos();
        ByteReader sec(bytes.data() + section_start, byte_length);

        parse_section(type, sec, base_dir, seen_meta);

        // Avanza el lector principal al final exacto del payload ya procesado.
        reader.skip(byte_length);
    }

    if (!seen_meta) {
        throw std::runtime_error("BinaryMapLoader: falta la seccion META");
    }
}

void BinaryMapLoader::parse_section(uint16_t type, ByteReader& section_reader,
                                    const std::string& base_dir, bool& seen_meta) {
    switch (static_cast<BinaryMapFormat::Section>(type)) {
        case BinaryMapFormat::Section::META:
            parse_meta_section(section_reader, seen_meta);
            break;
        case BinaryMapFormat::Section::TILESETS:
            parse_tilesets_section(section_reader, base_dir);
            break;
        case BinaryMapFormat::Section::LAYERS:
            parse_layers_section(section_reader, seen_meta);
            break;
        default:
            // Seccion desconocida: se ignora su payload para forward-compat.
            break;
    }
}

void BinaryMapLoader::parse_meta_section(ByteReader& section_reader, bool& seen_meta) {
    tile_size = static_cast<int>(section_reader.read_u32());
    width     = static_cast<int>(section_reader.read_u32());
    height    = static_cast<int>(section_reader.read_u32());
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("BinaryMapLoader: width/height invalidos en META");
    }
    seen_meta = true;
}

void BinaryMapLoader::parse_tilesets_section(ByteReader& section_reader,
                                             const std::string& base_dir) {
    uint32_t count = section_reader.read_u32();
    tilesets.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        Tileset ts;
        ts.name = section_reader.read_string();
        std::string rel = section_reader.read_string();
        ts.file_path = (std::filesystem::path(base_dir) / rel).string();
        ts.columns    = static_cast<int>(section_reader.read_u32());
        ts.tile_count = static_cast<int>(section_reader.read_u32());
        ts.firstgid   = static_cast<int>(section_reader.read_u32());
        ts.collidable = (section_reader.read_u8() != 0);
        tilesets.push_back(std::move(ts));
    }
}

void BinaryMapLoader::parse_layers_section(ByteReader& section_reader, bool seen_meta) {
    if (!seen_meta) {
        throw std::runtime_error("BinaryMapLoader: LAYERS antes que META");
    }

    uint32_t count = section_reader.read_u32();
    layers.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        MapLayerData layer;
        layer.name = section_reader.read_string();
        layer.data.reserve(height);
        for (int y = 0; y < height; ++y) {
            std::vector<int> row;
            row.reserve(width);
            for (int x = 0; x < width; ++x) {
                row.push_back(static_cast<int>(section_reader.read_u16()));
            }
            layer.data.push_back(std::move(row));
        }
        layers.push_back(std::move(layer));
    }
}

void BinaryMapLoader::build_tile_index() {
    // Identico a MapLoader::build_tile_index (mapLoader.cpp 91-103).
    for (int ts_idx = 0; ts_idx < static_cast<int>(tilesets.size()); ++ts_idx) {
        const Tileset& ts = tilesets[ts_idx];
        for (int local = 0; local < ts.tile_count; ++local) {
            TileDef td;
            td.id = ts.firstgid + local;
            td.tileset_index = ts_idx;
            td.local_index = local;
            td.collidable = ts.collidable;
            tiles[td.id] = td;
        }
    }
}

int BinaryMapLoader::get_tile_size() const { return tile_size; }
int BinaryMapLoader::get_width() const { return width; }
int BinaryMapLoader::get_height() const { return height; }

const std::vector<Tileset>& BinaryMapLoader::get_tilesets() const { return tilesets; }
const std::vector<MapLayerData>& BinaryMapLoader::get_layers() const { return layers; }
const std::map<std::string, positionCoord>& BinaryMapLoader::get_spawns() const {
    return spawns;
}
const std::vector<TeleportDef>& BinaryMapLoader::get_teleports() const {
    return teleports;
}

const TileDef* BinaryMapLoader::find_tile(int id) const {
    // Copia exacta de MapLoader::find_tile (mapLoader.cpp 31-36).
    if (id == 0) return nullptr;
    auto it = tiles.find(id);
    if (it == tiles.end()) return nullptr;
    return &it->second;
}

bool BinaryMapLoader::is_collidable(int x, int y) const {
    // Copia exacta de MapLoader::is_collidable (mapLoader.cpp 38-49): garantiza
    // que el server obtenga el mismo mundo desde un .bin que desde un .toml.
    if (x < 0 || y < 0 || x >= width || y >= height) return true;
    for (const auto& layer : layers) {
        if (y >= static_cast<int>(layer.data.size())) continue;
        const auto& row = layer.data[y];
        if (x >= static_cast<int>(row.size())) continue;
        int id = row[x];
        const TileDef* td = find_tile(id);
        if (td && td->collidable) return true;
    }
    return false;
}
