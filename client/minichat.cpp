#include "minichat.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

MiniChat::MiniChat(SDL_Renderer* renderer, TTF_Font* font) : 
    renderer(renderer),
    font(font) {}

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

SDL_Texture* MiniChat::create_texture_from_msg(const std::string& msg, float& width, float& height) {
    if (msg.empty()) {
        width = 0.0f;
        height = 0.0f;
        return nullptr;
    }

    SDL_Color msg_color = {255, 255, 255, 255};

    SDL_Surface* surface = TTF_RenderText_Blended(font, msg.c_str(), 0, msg_color);

    if (!surface) {
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    if (texture) {
        SDL_GetTextureSize(texture, &width, &height);
    }

    SDL_DestroySurface(surface);

    return texture;
}

void MiniChat::update(Queue<std::string>& inbox) {
    std::string msg;

    while (inbox.try_pop(msg)) {
        float width = 0.0f;
        float height = 0.0f;

        SDL_Texture* texture = create_texture_from_msg(msg, width, height);

        if (texture) {
            chatMessage new_msg;
            new_msg.text = msg;
            new_msg.texture = texture;
            new_msg.width = width;
            new_msg.height = height;

            msg_history.push_back(new_msg);

            if (msg_history.size() > MAX_LINES) {
                pop_oldest_message();
            }
        }
    }
}

void MiniChat::pop_oldest_message() {
    if (!msg_history.empty()) {
        if (msg_history.front().texture) {
            SDL_DestroyTexture(msg_history.front().texture);
        }

        msg_history.pop_front();
    }
}

bool MiniChat::has_pending_outbound_message() const {
    return !outbound_msg.empty();
}

std::string MiniChat::pop_outbound_message() {
    std::string msg = std::move(outbound_msg);

    outbound_msg.clear();

    return msg;
}

MiniChat::~MiniChat() {
    while (!msg_history.empty()) {
        pop_oldest_message();
    }

    TTF_CloseFont(font);
}
