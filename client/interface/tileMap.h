#ifndef TILE_MAP_H
#define TILE_MAP_H

#include <SDL3/SDL.h>
#include <string>
#include <vector>

#include "binaryMap/binaryMapLoader.h"

constexpr int LAYER_GROUND = 0;
constexpr int LAYER_BUILDINGS = 1;
constexpr int LAYER_ABOVE_PLAYER = 2;
constexpr int LAYER_LAST = -1;

class TileMap {
private:
    SDL_Renderer* renderer;
    BinaryMapLoader mapData;
    std::vector<SDL_Texture*> tileset_textures;

    // Carga en tileset_textures una textura por cada tileset ya parseado
    // en mapData
    void load_tileset_textures();

public:
    explicit TileMap(SDL_Renderer* renderer);
    ~TileMap();

    TileMap(const TileMap&) = delete;
    TileMap& operator=(const TileMap&) = delete;

    // Parsea el mapa binario
    // carga las texturas de cada tileset en el vector tileset_textures
    void load_map_bin(const std::string& path);

    // recorre las capas con índice en [first_layer, last_layer) en orden
    void render(int mapViewport_x, int mapViewport_y,
                int first_layer = 0, int last_layer = -1) const;

    int getTileSize() const;
    int getWidth() const;
    int getHeight() const;
    int getPixelWidth() const;
    int getPixelHeight() const;

};

#endif
