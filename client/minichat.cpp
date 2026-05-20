#include "minichat.h"

#include <SDL3/SDL.h>

MiniChat::MiniChat(SDL_Renderer* renderer) : renderer(renderer) {}

bool MiniChat::handle_event(const SDL_Event& event) {
    if (!is_active) {
        return false;
    }

    if (event.type == SDL_EVENT_KEY_DOWN) {
        switch (event.key.scancode) {
            case SDL_SCANCODE_ESCAPE:
                player_input.clear();
                toggle_active();
                break;
            case SDL_SCANCODE_KP_ENTER:
                if (!player_input.empty()) {
                    outbound_msg = player_input;
                    player_input.clear();
                }
                break;
            case SDL_SCANCODE_BACKSPACE:
                if (!player_input.empty()) {
                    player_input.pop_back();
                }
                break;
            default:
                break;
        }
    } else if (event.type == SDL_EVENT_TEXT_INPUT) {
        player_input += event.text.text;
    }

    return true;
}

void MiniChat::toggle_active() {
    is_active = !is_active;

    SDL_Window* window = SDL_GetRenderWindow(renderer);

    if (is_active) {
        player_input.clear();
        SDL_StartTextInput(window);
    } else {
        SDL_StopTextInput(window);
    }
}
