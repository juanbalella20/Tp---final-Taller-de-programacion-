#ifndef CLIENT_DISPLAY_H
#define CLIENT_DISPLAY_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include "displayPlayer.h"

#define WIN_NAME "Argentum"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct GameDisplay {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *background;
    PlayerDisplay player;
    SDL_Event event;
    bool is_running;
};

bool game_new(GameDisplay& g);
void game_run(GameDisplay& g);
void game_free(GameDisplay& g);

#endif  // CLIENT_DISPLAY_H
