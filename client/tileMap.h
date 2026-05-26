#ifndef TILE_MAP_H
#define TILE_MAP_H

#include <SDL3/SDL.h>
#include <string>
#include <vector>

#include "../common/mapLoader.h"

class TileMap {
private:
    SDL_Renderer* renderer;
    MapLoader mapData;
    std::vector<SDL_Texture*> tileset_textures;

public:
    explicit TileMap(SDL_Renderer* renderer);
    ~TileMap();

    TileMap(const TileMap&) = delete;
    TileMap& operator=(const TileMap&) = delete;

    // Parsea el TOML (via MapLoader) 
    // y carga las texturas de cada tileset en el vector
    // tileset_textures
    void load_map(const std::string& tomlPath);

    // recorre las capas en orden, y para cada celda no vacía calcula el 
    // recorte del spritesheet (src) y la posición en pantalla (dst)
    void render(int mapViewport_x, int mapViewport_y) const;

    int getTileSize() const;
    int getWidth() const;
    int getHeight() const;
    int getPixelWidth() const;
    int getPixelHeight() const;

    // Consultar colisiones (delegado a MapData::is_solid).
    bool isBlocked(int cellX, int cellY) const { return mapData.is_collidable(cellX, cellY); }

    const std::map<std::string, positionCoord>& getSpawns() const { return mapData.get_spawns(); }
};

#endif