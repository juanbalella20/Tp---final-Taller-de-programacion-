#ifndef NPC_SPRITE_H
#define NPC_SPRITE_H

#include <SDL3/SDL.h>
#include "worldEntity.h"

// Sprite para dibujar un NPC en una celda del mapa
class NpcSprite : public WorldEntity {
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    int tile_x;
    int tile_y;
    int tile_size;

public:
    NpcSprite(SDL_Renderer* renderer, SDL_Texture* texture,
              int tile_x, int tile_y, int tile_size);

    void draw(const Camera& camera, SDL_FRect src_crop) const override;
};

#endif
