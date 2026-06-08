#include "text_field.h"

#include "ui_helpers.h"

TextField::TextField(const SDL_FRect& box, bool password)
    : box_(box), password_(password) {}

void TextField::handle_event(const SDL_Event& event) {
    if (!focused_) {
        return;
    }

    if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.scancode == SDL_SCANCODE_BACKSPACE && !value_.empty()) {
            value_.pop_back();
        }
    } else if (event.type == SDL_EVENT_TEXT_INPUT) {
        value_ += event.text.text;
    }
}

void TextField::set_focus(bool focus) {
    // Solo cambia el flag. El SDL_StartTextInput/StopTextInput (estado global por
    // ventana) lo maneja la pantalla, no el campo.
    focused_ = focus;
}

bool TextField::is_focused() const { return focused_; }

void TextField::render(SDL_Renderer* renderer, TTF_Font* font) const {
    // Fondo del campo (oscuro semitransparente).
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 220);
    SDL_RenderFillRect(renderer, &box_);

    // Borde: mas claro cuando esta enfocado.
    if (focused_) {
        SDL_SetRenderDrawColor(renderer, 220, 200, 120, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 120, 120, 130, 255);
    }
    SDL_RenderRect(renderer, &box_);

    // Texto a mostrar: asteriscos si es campo de contrasena.
    std::string shown = password_ ? std::string(value_.size(), '*') : value_;

    // Cursor parpadeante (igual que minichat.cpp: alterna cada 500 ms).
    if (focused_ && (SDL_GetTicks() / 500) % 2 == 0) {
        shown += "_";
    }

    if (!shown.empty()) {
        const SDL_Color text_color{ 235, 235, 235, 255 };
        const float padding_x = 6.0f;
        const float text_x = box_.x + padding_x;
        // Centrado vertical aproximado dentro de la caja.
        const float text_y = box_.y + (box_.h * 0.5f) - 9.0f;
        draw_text(renderer, font, shown.c_str(), text_x, text_y, text_color);
    }
}

const std::string& TextField::value() const { return value_; }

void TextField::clear() { value_.clear(); }

bool TextField::hit(float x, float y) const { return contains(box_, x, y); }

const SDL_FRect& TextField::box() const { return box_; }
