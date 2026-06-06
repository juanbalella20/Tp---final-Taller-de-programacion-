#include "mapLoader.h"

#include <filesystem>
#include <stdexcept>
#include "../vendored/tomlplusplus/toml.hpp"
#include "binaryMap/binaryMapLoader.h"
#include <iostream>

int MapLoader::get_tile_size() const {
    return tile_size;
}
int MapLoader::get_width() const {
    return width;
}
int MapLoader::get_height() const {
    return height;
}

const std::map<std::string, positionCoord>& MapLoader::get_spawns() const {
    return spawns;
}

const std::vector<Tileset>& MapLoader::get_tilesets() const {
    return tilesets;
}

const std::vector<MapLayerData>& MapLoader::get_layers() const {
    return layers;
}


const TileDef* MapLoader::find_tile(int id) const {
    if (id == 0) return nullptr;
    auto it = tiles.find(id);
    if (it == tiles.end()) return nullptr;
    return &it->second;
}

bool MapLoader::is_collidable(int x, int y) const {
    // La grilla de colision es la unica fuente de verdad. Fuera del mapa siempre
    // bloquea; in-bounds, depende de la grilla (vacia/sin entrada = transitable).
    if (x < 0 || y < 0 || x >= width || y >= height) return true;
    if (y >= static_cast<int>(collision.size())) return false;
    const auto& row = collision[y];
    if (x >= static_cast<int>(row.size())) return false;
    return row[x] != 0;
}

void MapLoader::derive_collision_from_layers() {
    // Path TOML legacy: el TOML no trae grilla explicita, asi que la derivamos
    // del flag 'collidable' por-tileset de los tiles presentes en cada celda.
    collision.assign(height, std::vector<uint8_t>(width, 0));
    for (const auto& layer : layers) {
        for (int y = 0; y < height && y < static_cast<int>(layer.data.size()); ++y) {
            const auto& src = layer.data[y];
            for (int x = 0; x < width && x < static_cast<int>(src.size()); ++x) {
                const TileDef* td = find_tile(src[x]);
                if (td && td->collidable) collision[y][x] = 1;
            }
        }
    }
}


void MapLoader::parse_tilesets(const toml::table& tbl,
                               const std::filesystem::path& baseDir) {
    auto* arr = tbl["tileset"].as_array();
    if (!arr) {
        throw std::runtime_error("MapLoader: falta [[tileset]] en el TOML");
    }
    // recorre el array de tilesets
    for (auto& node : *arr) {
        auto* t = node.as_table();
        if (!t) continue;

        Tileset ts;
        ts.name          = (*t)["name"].value_or<std::string>("");
        // Guarda el path relativo del set (por ejemplo sand.png)
        std::string file = (*t)["file"].value_or<std::string>("");
        ts.columns       = (*t)["columns"].value_or(1);
        ts.tile_count    = (*t)["tile_count"].value_or(1);
        ts.firstgid      = (*t)["firstgid"].value_or(0);
        ts.collidable    = (*t)["collidable"].value_or(false);

        if (file.empty()) {
            throw std::runtime_error("MapLoader: tileset sin 'file'");
        }
        if (ts.columns <= 0 || ts.tile_count <= 0 || ts.firstgid <= 0) {
            throw std::runtime_error(
                "MapLoader: tileset '" + ts.name +
                "' tiene columns/tile_count/firstgid invalidos");
        }
        // convierte path relativo (file) a path absoluto (file_path)
        ts.file_path = (baseDir / file).string();

        tilesets.push_back(std::move(ts));
    }

    if (tilesets.empty()) {
        throw std::runtime_error("MapLoader: ningun [[tileset]] valido");
    }
}

void MapLoader::build_tile_index() {
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

void MapLoader::parse_layers(const toml::table& tbl) {
    // detecta que existan capas "layers" en el archivo
    auto* arr = tbl["layer"].as_array();
    // Debug print :
    std::cout << "[DEBUG]: tbl['layer']: " << arr << std::endl;
    //
    if (!arr) {
        throw std::runtime_error("MapLoader: falta [[layer]] en el TOML");
    }
    // itera cada layer
    for (auto& node : *arr) {
        auto* t = node.as_table();
        if (!t) continue;

        MapLayerData layer;
        // si esa capa no tien atributo 'name', se ignora
        layer.name = (*t)["name"].value_or<std::string>("");

        // lee la matriz 'data' del TOML
        auto* data_arr = (*t)["data"].as_array();
        if (!data_arr) {
            throw std::runtime_error(
                "MapLoader: capa '" + layer.name + "' sin 'data'");
        }

        layer.data.reserve(data_arr->size());
        for (auto& row_node : *data_arr) {
            auto* row_arr = row_node.as_array();
            if (!row_arr) {
                throw std::runtime_error(
                    "MapLoader: capa '" + layer.name +
                    "' tiene una fila que no es array");
            }
            std::vector<int> row;
            row.reserve(row_arr->size());
            for (auto& cell : *row_arr) {
                row.push_back(static_cast<int>(cell.value_or<int64_t>(0)));
            }
            layer.data.push_back(std::move(row));
        }

        // valida que la matriz coincida con el width y height del TOML
        if (static_cast<int>(layer.data.size()) != height) {
            throw std::runtime_error(
                "MapLoader: capa '" + layer.name + "' tiene " +
                std::to_string(layer.data.size()) +
                " filas, se esperaban " + std::to_string(height));
        }
        for (size_t r = 0; r < layer.data.size(); ++r) {
            if (static_cast<int>(layer.data[r].size()) != width) {
                throw std::runtime_error(
                    "MapLoader: capa '" + layer.name + "' fila " +
                    std::to_string(r) + " tiene " +
                    std::to_string(layer.data[r].size()) +
                    " columnas, se esperaban " + std::to_string(width));
            }
        }

        layers.push_back(std::move(layer));
    }
}

void MapLoader::parse_spawns(const toml::table& tbl) {
    auto* arr = tbl["spawn"].as_array();
    if (!arr) return;

    for (auto& node : *arr) {
        auto* t = node.as_table();
        if (!t) continue;

        std::string name = (*t)["name"].value_or<std::string>("");
        if (name.empty()) continue;
        positionCoord p{
            (*t)["x"].value_or(0),
            (*t)["y"].value_or(0),
        };
        spawns[name] = p;
    }
}

void MapLoader::parse_teleports(const toml::table& tbl) {
    auto* arr = tbl["teleport"].as_array();
    if (!arr) return;

    for (auto& node : *arr) {
        auto* t = node.as_table();
        if (!t) continue;

        std::string dest = (*t)["dest_zone"].value_or<std::string>("");
        if (dest.empty()) continue;
        TeleportDef tp{
            (*t)["x"].value_or(0),
            (*t)["y"].value_or(0),
            dest,
        };
        teleports.push_back(tp);
    }
}


void MapLoader::load(const std::string& tomlPath) {
    std::filesystem::path baseDir = std::filesystem::path(tomlPath).parent_path();

    toml::table tbl;
    try {
        tbl = toml::parse_file(tomlPath);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error(std::string("MapLoader: parse ") + tomlPath +
                                 ": " + std::string(e.description()));
    }

    tile_size = tbl["tile_size"].value_or(64);
    width     = tbl["width"].value_or(0);
    height    = tbl["height"].value_or(0);
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("MapLoader: width/height invalidos en " + tomlPath);
    }

    parse_tilesets(tbl, baseDir);
    build_tile_index();
    parse_layers(tbl);
    parse_spawns(tbl);
    parse_teleports(tbl);
    // La grilla se deriva DESPUES de tener tiles + capas.
    derive_collision_from_layers();
}
void MapLoader::load_bin(const std::string& binPath) {
    // BinaryMapLoader expone la misma superficie que MapLoader: parsea el .bin
    BinaryMapLoader bin;
    bin.load(binPath);

    tile_size = bin.get_tile_size();
    width     = bin.get_width();
    height    = bin.get_height();
    tilesets  = bin.get_tilesets();
    layers    = bin.get_layers();
    spawns    = bin.get_spawns();
    teleports = bin.get_teleports();
    collision = bin.get_collision();

    tiles.clear();
    build_tile_index();
}