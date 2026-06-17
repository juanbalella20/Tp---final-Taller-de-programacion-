#include "tileMap.h"

#include <SDL3_image/SDL_image.h>
#include <stdexcept>

#include "utility/paths.h"  // resolve_resource

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

void TileMap::load_tileset_textures() {
    // Al recargar (cambio de zona) hay que descartar las texturas anteriores:
    // si no, el vector acumula las viejas + nuevas y el render (que indexa por
    // tileset_index desde 0) dibuja con las del mapa previo. Ademas evita el leak.
    for (auto* tex : tileset_textures) {
        if (tex) SDL_DestroyTexture(tex);
    }
    tileset_textures.clear();

    const auto& tilesets = mapData.get_tilesets();
    tileset_textures.reserve(tilesets.size());
    for (const auto& ts : tilesets) {
        // file_path en el .bin es relativa a la carpeta de recursos compartida
        // (la que use el editor). La resolvemos contra NUESTRA carpeta de
        // recursos (ARGENTUM_RESOURCES_DIR, o el cwd del repo). Ver paths.h.
        const std::string img_path = paths::resolve_resource(ts.file_path);
        SDL_Texture* tex = IMG_LoadTexture(renderer, img_path.c_str());
        if (!tex) {
            throw std::runtime_error("TileMap: no pude cargar " + img_path +
                                     " (" + SDL_GetError() + ")");
        }
        // evitar suavizado al escalar
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
        tileset_textures.push_back(tex);
    }
}

void TileMap::load_map_toml(const std::string& tomlPath) {
    mapData.load(tomlPath);
    load_tileset_textures();
}

void TileMap::load_map_bin(const std::string& path) {
    mapData.load_bin(path);
    load_tileset_textures();
}

void TileMap::render(int mapViewport_x, int mapViewport_y) const {
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
                SDL_FRect dst{ (col * tsf)-mapViewport_x, (row * tsf)-mapViewport_y, tsf, tsf };
                SDL_RenderTexture(renderer,
                                  tileset_textures[td->tileset_index],
                                  &src, &dst);
            }
        }
    }
}