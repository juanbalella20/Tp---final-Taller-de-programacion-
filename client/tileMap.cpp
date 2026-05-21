#include "tileMap.h"

#include <SDL3_image/SDL_image.h>
#include <filesystem>
#include <stdexcept>

#include "../vendored/tomlplusplus/toml.hpp"

TileMap::TileMap(SDL_Renderer* renderer)
    : renderer(renderer), tileSize(0), width(0), height(0) {}

TileMap::~TileMap() {
    for (auto& l : layers) {
        if (l.texture) SDL_DestroyTexture(l.texture);
    }
    layers.clear();
}

void TileMap::load(const std::string& tomlPath) {
    std::filesystem::path baseDir = std::filesystem::path(tomlPath).parent_path();

    toml::table tbl;
    // Se abre el TOML file
    try {
        tbl = toml::parse_file(tomlPath);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error(std::string("TileMap: parse ") + tomlPath +
                                 ": " + std::string(e.description()));
    }

    // 
    tileSize = tbl["tile_size"].value_or(64);
    width    = tbl["width"].value_or(0);
    height   = tbl["height"].value_or(0);
    if (width <= 0 || height <= 0) {
        throw std::runtime_error("TileMap: width/height invlidos en " + tomlPath);
    }

    // [[layer]] — array de tablas con {file, collider?}
    if (auto* arr = tbl["layer"].as_array()) {
        for (auto& node : *arr) {
            auto* layerTbl = node.as_table();
            if (!layerTbl) continue;

            std::string file = (*layerTbl)["file"].value_or<std::string>("");
            if (file.empty()) continue;
            
            // abro el path de la imagen
            std::filesystem::path imgPath = baseDir / file;
            SDL_Texture* tex = IMG_LoadTexture(renderer, imgPath.string().c_str());
            if (!tex) {
                throw std::runtime_error("TileMap: no pude cargar " +
                                         imgPath.string() + " (" + SDL_GetError() + ")");
            }

            MapLayer ml{tex, (*layerTbl)["collider"].value_or(false)};
            layers.push_back(ml);
        }
    }

    // [[collider]] — array de tablas con {x, y, w, h}
    if (auto* arr = tbl["collider"].as_array()) {
        for (auto& node : *arr) {
            auto* t = node.as_table();
            if (!t) continue;
            ColliderRect c{
                (*t)["x"].value_or(0),
                (*t)["y"].value_or(0),
                (*t)["w"].value_or(1),
                (*t)["h"].value_or(1),
            };
            colliders.push_back(c);
        }
    }

    // [[spawn]] — array de tablas con {name, x, y}
    if (auto* arr = tbl["spawn"].as_array()) {
        for (auto& node : *arr) {
            auto* t = node.as_table();
            if (!t) continue;
            SpawnPoint s{
                (*t)["name"].value_or<std::string>(""),
                (*t)["x"].value_or(0),
                (*t)["y"].value_or(0),
            };
            spawns.push_back(std::move(s));
        }
    }
}

void TileMap::render() const {
    for (const auto& l : layers) {
        if (l.texture) {
            SDL_RenderTexture(renderer, l.texture, nullptr, nullptr);
        }
    }
}

// TODO
bool TileMap::isBlocked(int cellX, int cellY) {
    return true;
}