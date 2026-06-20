#ifndef TILE_MAP_H
#define TILE_MAP_H

#include <SDL3/SDL.h>
#include <string>
#include <vector>

#include "binaryMap/binaryMapLoader.h"

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

    // recorre las capas en orden, y para cada celda no vacía calcula el 
    // recorte del spritesheet (src) y la posición en pantalla (dst)
    void render(int mapViewport_x, int mapViewport_y) const;

    int getTileSize() const;
    int getWidth() const;
    int getHeight() const;
    int getPixelWidth() const;
    int getPixelHeight() const;

};

#endif