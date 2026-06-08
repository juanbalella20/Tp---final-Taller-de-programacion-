#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

// Helpers de UI reutilizables por las pantallas (login, creacion de personaje).
// Replican el patron que ya usan welcome_screen.cpp / minichat.cpp / hud.cpp

// Verifica si el punto (x, y) cae dentro del rectangulo
bool contains(const SDL_FRect& rect, float x, float y);

// Dibuja text en (x, y) con el color dado, usando font. Crea y destruye la
// textura en el momento (igual que WelcomeScreen::drawText). No hace nada si el
// font es nulo o el render falla.
void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* text,
               float x, float y, SDL_Color color);

// Igual que draw_text pero devuelve por referencia el ancho/alto en pixeles del
// texto renderizado (0 si no se pudo dibujar). Util para centrar o alinear.
void draw_text_sized(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                     float x, float y, SDL_Color color, float& out_w, float& out_h);

#endif  // UI_HELPERS_H
