#include "npcSprite.h"


NpcSprite::NpcSprite(SDL_Renderer* renderer, SDL_Texture* texture,
              int tile_x, int tile_y, int tile_size)
        : renderer(renderer), texture(texture),
          tile_x(tile_x), tile_y(tile_y), tile_size(tile_size) {}

void NpcSprite::draw(const Camera& camera, SDL_FRect src_crop) {
    const float wx = static_cast<float>(tile_x * tile_size);
    const float wy = static_cast<float>(tile_y * tile_size);
    SDL_FRect dst{
        camera.world_to_screen_x(wx),
        camera.world_to_screen_y(wy),
        static_cast<float>(tile_size),
        static_cast<float>(tile_size)
    };
    // Si el crop tiene area, recorta ese tile del spritesheet 
    // si viene vacio, dibuja la textura completa.
    // TODO: hacer movimientos como en displayPlayer.cpp
    const bool has_crop = src_crop.w > 0.0f && src_crop.h > 0.0f;
    SDL_RenderTexture(renderer, texture, has_crop ? &src_crop : nullptr, &dst);
}