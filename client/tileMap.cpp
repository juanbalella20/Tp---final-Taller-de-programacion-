#include "tileMap.h"

#include <SDL3_image/SDL_image.h>
#include <stdexcept>

TileMap::TileMap(SDL_Renderer* renderer)
    : renderer(renderer), mapData() {}

TileMap::~TileMap() {
    for (auto* tex : tileset_textures) {
        if (tex) SDL_DestroyTexture(tex);
    }
    tileset_textures.clear();
}

int TileMap::getTileSize() const {
    return mapData.get_tile_size();
}
int TileMap::getWidth() const {
    return mapData.get_width();
}
int TileMap::getHeight() const {
    return mapData.get_height();
}
int TileMap::getPixelWidth() const {
    return mapData.get_width() * mapData.get_tile_size();
}
int TileMap::getPixelHeight() const {
    return mapData.get_height() * mapData.get_tile_size();
}



void TileMap::load_map(const std::string& tomlPath) {
    mapData.load(tomlPath);

    // carga texturas
    const auto& tilesets = mapData.get_tilesets();
    tileset_textures.reserve(tilesets.size());
    for (const auto& ts : tilesets) {
        SDL_Texture* tex = IMG_LoadTexture(renderer, ts.file_path.c_str());
        if (!tex) {
            throw std::runtime_error("TileMap: no pude cargar " +
                                     ts.file_path + " (" + SDL_GetError() + ")");
        }
        // evitar suavizado al escalar
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
        tileset_textures.push_back(tex);
    }
}

void TileMap::render() const {
    const int ts = mapData.get_tile_size();
    if (ts <= 0) return;

    const int w = mapData.get_width();
    const int h = mapData.get_height();
    const float tsf = static_cast<float>(ts);   // version float para los SDL_FRect
    const auto& tilesets = mapData.get_tilesets();

    // itera las layers siguiendo el orden del TOML:
    // 1ero la capa del fondo y por ultimo la capa de frente (algoritmo del pintor)
    for (const auto& layer : mapData.get_layers()) {
        for (int row = 0; row < h; ++row) {
            if (row >= static_cast<int>(layer.data.size())) continue;
            const auto& data_row = layer.data[row];
            for (int col = 0; col < w; ++col) {
                if (col >= static_cast<int>(data_row.size())) continue;
                int id = data_row[col];
                const TileDef* td = mapData.find_tile(id);
                if (!td) continue;

                const Tileset& ts_def = tilesets[td->tileset_index];
                const int local = td->local_index;
                const float sx = (local % ts_def.columns) * tsf;
                const float sy = (local / ts_def.columns) * tsf;

                SDL_FRect src{ sx, sy, tsf, tsf };
                SDL_FRect dst{ col * tsf, row * tsf, tsf, tsf };
                SDL_RenderTexture(renderer,
                                  tileset_textures[td->tileset_index],
                                  &src, &dst);
            }
        }
    }
}