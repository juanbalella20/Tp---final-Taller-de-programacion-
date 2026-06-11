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

    // El crop define qué región del spritesheet se recorta. En pantalla se dibuja
    // a ese mismo tamaño, SALVO que no entre en el tile: ahí se achica al tile
    // manteniendo la relación de aspecto (caso de los PNG de sprite único, p. ej.
    // 1024x1024). Los íconos chicos (espada, poción, ~30px) quedan igual que antes.
    float dest_w = static_cast<float>(tile_size);
    float dest_h = static_cast<float>(tile_size);
    if (src_crop.w > 0.0f && src_crop.h > 0.0f) {
        float scale = SDL_min(1.0f, SDL_min(tile_size / src_crop.w,
                                            tile_size / src_crop.h));
        dest_w = src_crop.w * scale;
        dest_h = src_crop.h * scale;
    }

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
