#ifndef BINARY_MAP_LOADER_H
#define BINARY_MAP_LOADER_H

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "constants/game_constants.h"  // positionCoord
#include "mapLoader.h"                  // Tileset, TileDef, MapLayerData, TeleportDef

class ByteReader;

/*
 * Lee un mapa en formato binario .bin (ver binaryMapFormat.h).
 *
 * Expone DELIBERADAMENTE la MISMA superficie publica que MapLoader, de modo que
 * el server (ZoneWorld::load_terrain) pueda consumir un .bin con un cambio
 * minimo (eligo el loader por extension del archivo). No depende del editor.
 *

 */
class BinaryMapLoader {
public:
    /*
     * Abre y parsea el archivo. Valida magic, format_version y el marcador de
     * endianness. Recorre la tabla de secciones: parsea las conocidas y saltea
     * las desconocidas (forward-compat). Al terminar arma el indice de tiles.
     *
     * Lanza std::runtime_error si el archivo es invalido, de una version mas
     * nueva, esta truncado/corrupto, o ante error de I/O.
     */
    void load(const std::string& path);

    // --- Misma superficie que MapLoader -----------------------------------
    int get_tile_size() const;
    int get_width() const;
    int get_height() const;
    const std::vector<Tileset>& get_tilesets() const;
    const std::vector<MapLayerData>& get_layers() const;
    const std::map<std::string, positionCoord>& get_spawns() const;
    const std::vector<TeleportDef>& get_teleports() const;

    // Devuelve nullptr si el id es 0 o no esta registrado en ningun tileset.
    const TileDef* find_tile(int id) const;

    // True si la celda esta fuera del mapa, o si alguna capa tiene en (x,y) un
    // tile colidable. Misma logica exacta que MapLoader::is_collidable.
    bool is_collidable(int x, int y) const;

private:
    int tile_size = 0;
    int width = 0;
    int height = 0;
    std::vector<Tileset> tilesets;
    std::unordered_map<int, TileDef> tiles;  // id global -> TileDef
    std::vector<MapLayerData> layers;
    std::map<std::string, positionCoord> spawns;
    std::vector<TeleportDef> teleports;

    std::vector<uint8_t> read_file(const std::string& path) const;
    void reset_state();
    uint32_t read_header(ByteReader& reader) const;
    void parse_sections(ByteReader& reader, const std::vector<uint8_t>& bytes,
                        const std::string& base_dir, uint32_t section_count);
    void parse_section(uint16_t type, ByteReader& section_reader,
                       const std::string& base_dir, bool& seen_meta);
    void parse_meta_section(ByteReader& section_reader, bool& seen_meta);
    void parse_tilesets_section(ByteReader& section_reader, const std::string& base_dir);
    void parse_layers_section(ByteReader& section_reader, bool seen_meta);

    // Recorre el vector de tilesets y arma el mapa <id, TileDef>.
    // Identico a MapLoader::build_tile_index.
    void build_tile_index();
};

#endif  // BINARY_MAP_LOADER_H
