#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

struct positionCoord {
    int x;
    int y;
};

struct sectorPerimiter {
    positionCoord top_left;
    positionCoord top_right;
    positionCoord bottom_left;
    positionCoord bottom_right;
};

struct groundGold {
    positionCoord pos;
    int amount;
};

enum class elements {
    buildings,
    empty,
    npcs
};

#define HEIGHT 16
#define WIDTH 30
// Tamano de tile en pixeles. Lo usan el editor grafico, el formato .bin y el
// render. Fijo en V1 (mapas 30x16 con tiles de 64x64).
#define TILE_SIZE 64


#endif