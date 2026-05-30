#include "itemSprite.h"

ItemSprite::ItemSprite(SDL_Renderer* renderer, SDL_Texture* texture, int tile_x, 
    int tile_y, int tile_size):
            renderer(renderer), 
            texture(texture), 
            tile_x(tile_x), 
            tile_y(tile_y), 
            tile_size(tile_size) {}

void ItemSprite::draw(const Camera& camera, SDL_FRect src_crop) const {
    const float wx = static_cast<float>(tile_x * tile_size);
    const float wy = static_cast<float>(tile_y * tile_size);

    SDL_FRect dest {
        camera.world_to_screen_x(wx),
        camera.world_to_screen_y(wy),
        static_cast<float>(tile_size),
        static_cast<float>(tile_size)
    };

    const SDL_FRect* src_rect = nullptr;
    if (src_crop.w > 0.0f && src_crop.h > 0.0f) {
        src_rect = &src_crop;
    }

    SDL_RenderTexture(renderer, texture, src_rect, &dest);
}
