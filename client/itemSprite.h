#ifndef ITEM_SPRITE_H_
#define ITEM_SPRITE_H_

#include <SDL3/SDL.h>
#include "worldEntity.h"

// Sprite para dibujar un Item en una celda del mapa
class ItemSprite : public WorldEntity {
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int tile_x;
    int tile_y;
    int tile_size;
    SDL_FRect src_crop;

public:
    ItemSprite(SDL_Renderer* renderer, SDL_Texture* texture,
              int tile_x, int tile_y, int tile_size, SDL_FRect crop);

    void draw(const Camera& camera) const override;
};


#endif
