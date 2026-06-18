#include "itemSprite.h"

ItemSprite::ItemSprite(SDL_Renderer* renderer, SDL_Texture* texture, int tile_x, 
    int tile_y, int tile_size):
            renderer(renderer), 
            texture(texture), 
            tile_x(tile_x), 
            tile_y(tile_y), 
            tile_size(tile_size) {}

void ItemSprite::draw(const Camera& camera, SDL_FRect src_crop) {
    const float wx = static_cast<float>(tile_x * tile_size);
    const float wy = static_cast<float>(tile_y * tile_size);

    float tex_w = 0.0f;
    float tex_h = 0.0f;

    if (src_crop.w > 0.0f && src_crop.h > 0.0f) {
        tex_w = src_crop.w;
        tex_h = src_crop.h;
    } else {
        SDL_GetTextureSize(texture, &tex_w, &tex_h);
    }

    const float SIZE_FACTOR = 0.6f; 
    float max_size = static_cast<float>(tile_size) * SIZE_FACTOR;
    float scale = SDL_min(max_size / tex_w, max_size / tex_h);
    // Opcional: Evitar que ítems minúsculos se estiren y se vean borrosos.
    // Si querés que los íconos muy chiquitos mantengan su tamaño original, descomentá esto:
    // scale = SDL_min(1.0f, scale); 
    float dest_w = tex_w * scale;
    float dest_h = tex_h * scale;
    SDL_FRect dest {
        camera.world_to_screen_x(wx + (tile_size - dest_w) / 2.0f),
        camera.world_to_screen_y(wy + (tile_size - dest_h) / 2.0f),
        dest_w,
        dest_h
    };

    const SDL_FRect* src_rect = nullptr;
    if (src_crop.w > 0.0f && src_crop.h > 0.0f) {
        src_rect = &src_crop;
    }

    SDL_RenderTexture(renderer, texture, src_rect, &dest);
}
