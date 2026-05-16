#include "clientGUI.h"
#include <SDL3/SDL_main.h>
#include <iostream>

// inicializa SDL, window y renderer
bool game_init_sdl(GameDisplay& g) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "Error initializing SDL3" << std::endl;
        return false;
    }
    g.window = SDL_CreateWindow(WIN_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    if (!g.window) {
        std::cout << "Error creating Window" << std::endl;
        return false;
    }
    g.renderer = SDL_CreateRenderer(g.window, NULL);
    if (!g.renderer) {
        std::cout << "Error creating Render" << std::endl;
        return false;
    } 

    SDL_Surface *icon_surf = IMG_Load("images/logo.jpeg");
    if (icon_surf) {
        SDL_SetWindowIcon(g.window, icon_surf);
        SDL_DestroySurface(icon_surf);
    }

    return true;
}

// creates background !
bool game_load_media(GameDisplay& g) {
    g.background = IMG_LoadTexture(g.renderer, "images/background.jpg");
    if (!g.background) {
        fprintf(stderr, "Error loading Texture: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

// libera los recursos de windows y renderer, cierra el SDL
void game_free(GameDisplay& g) {

    player_free(g.player);

    if(g.background) {
        SDL_DestroyTexture(g.background);
        g.background = NULL;
    }

    if (g.window) {
        SDL_DestroyWindow(g.window); 
        g.window = NULL;
    }
    if (g.renderer) {
        SDL_DestroyRenderer(g.renderer); 
        g.renderer = NULL;
    }
    SDL_Quit();

}

// analiza los eventos que son enviados desde el usuario
void game_events(GameDisplay& g) {
    while (SDL_PollEvent(&g.event)) {
        switch (g.event.type)
        {
        case SDL_EVENT_QUIT:
            g.is_running = false;
            break;
        case SDL_EVENT_KEY_DOWN :
            switch (g.event.key.scancode)
            {
            case SDL_SCANCODE_ESCAPE:
                g.is_running = false;
                break;            
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
}

void game_update(GameDisplay &g) {
    player_update(g.player);
}

// dibuja el renderer
void game_draw(GameDisplay& g) {
    SDL_RenderClear(g.renderer);
    
    SDL_RenderTexture(g.renderer, g.background, NULL, NULL);
    player_draw(g.player);

    SDL_RenderPresent(g.renderer);
}

// si no esta inicializado, setea is_running a true
bool game_new(GameDisplay& g) {
    if (!game_init_sdl(g)) {
        return false;
    }
    if (!game_load_media(g)) {
        return false;
    }
    if (!player_new(g.player, g.renderer)) {
        return false;
    }

    g.is_running = true;
    return true;
}

// mientras este corriendo, se dibuja y captura eventos
void game_run(GameDisplay& g) {
    SDL_Delay(100);
    while (g.is_running) {
        game_events(g);
        game_update(g);
        game_draw(g);
        SDL_Delay(16);
    }

}

int main(void) {
    bool exit_status = EXIT_FAILURE;

    GameDisplay game = {0};

    if (game_new(game)) {
        game_run(game);
        exit_status = EXIT_SUCCESS;
    }

    game_free(game);
    return exit_status;
}


