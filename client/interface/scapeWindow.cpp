#include "scapeWindow.h"

#include <SDL3_image/SDL_image.h>
#include <iostream>
#include "ui_helpers.h"
#include "paths.h"

ScapeWindow::ScapeWindow(SDL_Renderer *renderer, int logical_w, int logical_h)
    : renderer(renderer), logical_w(logical_w), logical_h(logical_h) {
    
  std::string path = paths::asset("imagenes/exit.png");
  SDL_Surface *surf = IMG_Load(path.c_str());
  if (surf) {
    background = SDL_CreateTextureFromSurface(renderer, surf);
    if (background)
      SDL_SetTextureBlendMode(background, SDL_BLENDMODE_BLEND);
    SDL_DestroySurface(surf);
  }
}

ScapeWindow::~ScapeWindow() {
  if (background)
    SDL_DestroyTexture(background);
}

void ScapeWindow::open() {
  open_ = true;
  exit_confirmed_ = false;
}

void ScapeWindow::close() { open_ = false; }

bool ScapeWindow::consume_exit_confirmed() {
  bool result = exit_confirmed_;
  exit_confirmed_ = false;
  return result;
}

SDL_FRect ScapeWindow::frame_rect() const {
  float h = logical_h * 0.5f;
  float w = h * (ART_W / ART_H);
  if (w > logical_w * 0.9f) {
    w = logical_w * 0.9f;
    h = w * (ART_H / ART_W);
  }
  float x = (logical_w - w) / 2.0f;
  float y = (logical_h - h) / 2.0f;
  return {x, y, w, h};
}

SDL_FRect ScapeWindow::cancel_button_rect() const {
  SDL_FRect frame = frame_rect();
  return {frame.x, frame.y + frame.h * (660.0f / ART_H), frame.w * 0.5f,
          frame.h * (130.0f / ART_H)};
}

SDL_FRect ScapeWindow::exit_button_rect() const {
  SDL_FRect frame = frame_rect();
  return {frame.x + frame.w * 0.5f, frame.y + frame.h * (660.0f / ART_H),
          frame.w * 0.5f, frame.h * (130.0f / ART_H)};
}

bool ScapeWindow::handle_event(const SDL_Event &event) {
  if (!open_)
    return false;

  if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
    close();
    return true;
  }
  if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
      event.button.button == SDL_BUTTON_LEFT) {
    float lx, ly;
    SDL_RenderCoordinatesFromWindow(renderer, event.button.x, event.button.y,
                                    &lx, &ly);

    if (contains(exit_button_rect(), lx, ly)) {
      exit_confirmed_ = true;
      close();
      return true;
    }
    if (contains(cancel_button_rect(), lx, ly)) {
      close();
      return true;
    }
    return true;
  }
  if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
      event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
    return true;
  }
  return false;
}

void ScapeWindow::render() {
  if (!open_)
    return;

  SDL_FRect frame = frame_rect();

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
  SDL_FRect full = {0, 0, static_cast<float>(logical_w),
                    static_cast<float>(logical_h)};
  SDL_RenderFillRect(renderer, &full);

  if (background) {
    SDL_RenderTexture(renderer, background, nullptr, &frame);
  } else {
    SDL_SetRenderDrawColor(renderer, 30, 25, 20, 240);
    SDL_RenderFillRect(renderer, &frame);
  }
}
