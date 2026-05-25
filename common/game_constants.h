#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

struct position_coord {
    int x;
    int y;
};

struct sectorPerimiter {
    position_coord top_left;
    position_coord top_right;
    position_coord bottom_left;
    position_coord bottom_right;
};

enum class elements {
    players,
    npcs,
    objects,
    buildings,
    empty
};

#define HEIGHT 16
#define WIDTH 30


#endif