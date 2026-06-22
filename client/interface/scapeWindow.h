#ifndef SCAPE_WINDOW_H
#define SCAPE_WINDOW_H

#include <SDL3/SDL.h>

class ScapeWindow {
public:
  ScapeWindow(SDL_Renderer *renderer, int logical_w, int logical_h);
  ~ScapeWindow();

  ScapeWindow(const ScapeWindow &) = delete;
  ScapeWindow &operator=(const ScapeWindow &) = delete;

  void open();
  void close();

  // true si el usuario confirmo "Salir" en este frame (se resetea al leerlo).
  bool consume_exit_confirmed();

  bool handle_event(const SDL_Event &event);

  void render();

private:
  SDL_Renderer *renderer;
  int logical_w;
  int logical_h;

  bool open_ = false;
  bool exit_confirmed_ = false;

  static constexpr float ART_W = 1263.0f;
  static constexpr float ART_H = 896.0f;

  SDL_FRect frame_rect() const;
  SDL_FRect exit_button_rect() const;
  SDL_FRect cancel_button_rect() const;
};

#endif
