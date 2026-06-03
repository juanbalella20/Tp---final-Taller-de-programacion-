#include "minichat.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

MiniChat::MiniChat(SDL_Renderer* renderer, TTF_Font* font) : 
    active(false),
    renderer(renderer),
    font(font) {}

bool MiniChat::is_active() const { return active; }

bool MiniChat::handle_event(const SDL_Event& event) {
    if (!active) {
        return false;
    }

    if (event.type == SDL_EVENT_QUIT) {
        return false;
    }

    if (event.type == SDL_EVENT_KEY_DOWN) {
        switch (event.key.scancode) {
            case SDL_SCANCODE_ESCAPE:
                player_input.clear();
                toggle_active();
                break;
            case SDL_SCANCODE_KP_ENTER:
            case SDL_SCANCODE_RETURN:
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
    active = !active;

    SDL_Window* window = SDL_GetRenderWindow(renderer);

    if (active) {
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

void MiniChat::render(int game_width, int game_height) {
    float panel_x = 10.0f;
    float panel_y = 10.0f;

    float line_h  = 20.0f;
    float spacing = 3.0f;
    float input_h = active ? (line_h + 6.0f) : 0.0f;

    float panel_w = static_cast<float>(game_width) - 20.0f;
    float panel_h = (MAX_LINES/2) * (line_h + spacing) + 10.0f + input_h;

    Uint8 alpha = active ? 255 : 180;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_FRect border_rect = {
        panel_x - 2.0f, panel_y - 2.0f, panel_w + 4.0f, panel_h + 4.0f
    };
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, alpha);
    SDL_RenderFillRect(renderer, &border_rect);

    SDL_FRect bg_rect = { panel_x, panel_y, panel_w, panel_h };
    SDL_SetRenderDrawColor(renderer, 10, 10, 10, alpha);
    SDL_RenderFillRect(renderer, &bg_rect);

    float start_x = panel_x + 8.0f;
    float current_y = panel_y + 5.0f;

    for (const auto& msg : msg_history) {
        SDL_FRect dest_rect = { start_x, current_y, msg.width, msg.height };
        SDL_RenderTexture(renderer, msg.texture, nullptr, &dest_rect);
        current_y += msg.height + spacing;
    }

    if (active) {
        SDL_FRect separator = {
            panel_x, current_y - 2.0f, panel_w, 2.0f
        };
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, alpha);
        SDL_RenderFillRect(renderer, &separator);

        std::string prompt = "> " + player_input;

        if ((SDL_GetTicks() / 500) % 2 == 0) {
            prompt += "_";
        }

        float input_width = 0.0f;
        float input_height = 0.0f;
        SDL_Texture* input_texture = create_texture_from_msg(prompt, input_width, input_height);

        if (input_texture) {
            SDL_FRect dest_rect = { start_x, current_y + 3.0f, input_width, input_height };
            SDL_RenderTexture(renderer, input_texture, nullptr, &dest_rect);

            SDL_DestroyTexture(input_texture);
        }
    }
}

MiniChat::~MiniChat() {
    while (!msg_history.empty()) {
        pop_oldest_message();
    }
}
