#ifndef TILE_MAP_H
#define TILE_MAP_H

#include <SDL3/SDL.h>
#include <string>
#include <vector>

struct MapLayer {
    SDL_Texture* texture;
    bool collider;
};

struct ColliderRect {
    int x, y, w, h;  // en celdas (no en pxeles)
};

struct SpawnPoint {
    std::string name;
    int x, y;  // en celdas
};

class TileMap {
private:
    SDL_Renderer* renderer;
    int tileSize;
    int width;   // ancho en celdas
    int height;  // alto en celdas
    std::vector<MapLayer> layers;
    std::vector<ColliderRect> colliders;
    std::vector<SpawnPoint> spawns;

public:
    explicit TileMap(SDL_Renderer* renderer);
    ~TileMap();

    TileMap(const TileMap&) = delete;
    TileMap& operator=(const TileMap&) = delete;

    // Carga un mapa desde un .toml. Las rutas de los PNG en el TOML son
    // relativas al directorio del .toml.
    void load(const std::string& tomlPath);

    // Dibuja todas las capas en el orden en el que estn en el TOML.
    void render() const;

    int getTileSize()  const { return tileSize; }
    int getWidth()     const { return width; }
    int getHeight()    const { return height; }
    int getPixelWidth()  const { return width * tileSize; }
    int getPixelHeight() const { return height * tileSize; }

    bool isBlocked(int cellX, int cellY);

    const std::vector<ColliderRect>& getColliders() const { return colliders; }
    const std::vector<SpawnPoint>&   getSpawns()    const { return spawns; }
};

#endif
