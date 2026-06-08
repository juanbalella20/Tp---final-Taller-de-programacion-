#include "ui_helpers.h"

bool contains(const SDL_FRect& rect, float x, float y) {
    return x >= rect.x && x <= rect.x + rect.w &&
           y >= rect.y && y <= rect.y + rect.h;
}

void draw_text_sized(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                     float x, float y, SDL_Color color, float& out_w, float& out_h) {
    out_w = 0.0f;
    out_h = 0.0f;
    if (!font || !text) {
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(font, text, 0, color);
    if (!surface) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_DestroySurface(surface);
        return;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    out_w = static_cast<float>(surface->w);
    out_h = static_cast<float>(surface->h);
    SDL_FRect dst{ x, y, out_w, out_h };
    SDL_DestroySurface(surface);
    SDL_RenderTexture(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* text,
               float x, float y, SDL_Color color) {
    float w = 0.0f;
    float h = 0.0f;
    draw_text_sized(renderer, font, text, x, y, color, w, h);
}
