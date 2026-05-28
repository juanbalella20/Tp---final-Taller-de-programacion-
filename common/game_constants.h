#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

#include <memory>
#include "../server/item.h"

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

struct groundItem {
    positionCoord pos;
    std::unique_ptr<Item> item;
};

struct groundGold {
    positionCoord pos;
    int amount;
};

enum class elements {
    buildings,
    empty
};

#define HEIGHT 16
#define WIDTH 30


#endif