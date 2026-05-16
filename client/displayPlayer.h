#ifndef DISPLAY_PLAYER_H
#define DISPLAY_PLAYER_H

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#define PLAYER_VEL 3.0f

struct PlayerDisplay {
    SDL_Renderer *renderer;
    SDL_Texture *image;
    SDL_FRect rect;
    const bool *keystate;
};
 /*
  * Constructor de Player display.
  * Guarda el renderer
  * Carga la image (from disk into a surface)
  * Convierte la surface en texture
  * Lee el texture width/height y lo guarda
  * Setea posicion initial
  * Toma el estado del teclado
  */
bool player_new(PlayerDisplay& player, SDL_Renderer* renderer);
// libera los recursos del spray
void player_free(PlayerDisplay& player);
// se encarga del movimiento del jugador
void player_update(PlayerDisplay& player);
// le indica al SDL pintar la textura del player por pantalla
void player_draw(const PlayerDisplay& player);

#endif
