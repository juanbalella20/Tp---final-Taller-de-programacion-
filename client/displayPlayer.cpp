#include "displayPlayer.h"
#include <cstdio>

bool player_new(PlayerDisplay& p, SDL_Renderer* renderer) {
    p.renderer = renderer;

    SDL_Surface *surf = IMG_Load("images/player.png");
    if (!surf) {
        fprintf(stderr, "Error loading player surface: %s\n", SDL_GetError());
        return false;
    }

    p.image = SDL_CreateTextureFromSurface(p.renderer, surf);
    SDL_DestroySurface(surf);
    if (!p.image) {
        fprintf(stderr, "Error creating player texture: %s\n", SDL_GetError());
        return false;
    }

    if (!SDL_GetTextureSize(p.image, &p.rect.w, &p.rect.h)) {
        fprintf(stderr, "Error getting texture size: %s\n", SDL_GetError());
        return false;
    }

    p.rect.w = 64.0f;
    p.rect.h = 64.0f;
    p.rect.x = 64.0f;
    p.rect.y = 64.0f;
    p.keystate = SDL_GetKeyboardState(NULL);
    return true;
}

void player_free(PlayerDisplay& p) {
    if (p.image) {
        SDL_DestroyTexture(p.image);
        p.image = NULL;
    }
    p.renderer = NULL;
    p.keystate = NULL;
}

void player_update(PlayerDisplay& p) {
    if (p.keystate[SDL_SCANCODE_LEFT] || p.keystate[SDL_SCANCODE_A])
        p.rect.x -= PLAYER_VEL;
    if (p.keystate[SDL_SCANCODE_RIGHT] || p.keystate[SDL_SCANCODE_D])
        p.rect.x += PLAYER_VEL;
    if (p.keystate[SDL_SCANCODE_UP] || p.keystate[SDL_SCANCODE_W])
        p.rect.y -= PLAYER_VEL;
    if (p.keystate[SDL_SCANCODE_DOWN] || p.keystate[SDL_SCANCODE_S])
        p.rect.y += PLAYER_VEL;
}

void player_draw(const PlayerDisplay& p) {
    SDL_RenderTexture(p.renderer, p.image, NULL, &p.rect);
}
