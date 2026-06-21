#include "minichat.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "paths.h"

MiniChat::MiniChat(SDL_Renderer* renderer, float game_width, float panel_width, float canvas_height) : 
    active(false),
    renderer(renderer),
    game_width(game_width),
    panel_width(panel_width),
    canvas_height(canvas_height) {

    const std::string font_path = paths::asset("fonts/StackSansText-Medium.ttf");
    font = TTF_OpenFont(font_path.c_str(), 12);
}

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
    float image_w = 1021.0f;
    float image_h = 767.0f;

    float scale_x = static_cast<float>(game_width + panel_width) / image_w;
    float scale_y = static_cast<float>(canvas_height) / image_h;

    float panel_x = 11.0f * scale_x;
    float panel_y = 33.0f * scale_y;
    float panel_w = 619.0f * scale_x;
    float panel_h = 86.0f * scale_y;

    float line_h  = 20.0f * scale_y;
    float spacing = 3.0f * scale_y;

    float start_x = panel_x + (5.0f * scale_x);
    float input_y = panel_y + panel_h - line_h - (5.0f * scale_y);
    float available_h = active ? (input_y - panel_y) : panel_h;
    int visible_lines = static_cast<int>(available_h / (line_h + spacing));
    if (visible_lines < 1) visible_lines = 1;

    int total_msgs = static_cast<int>(msg_history.size());
    int start_idx = (total_msgs > visible_lines) ? (total_msgs - visible_lines) : 0;

    float current_y = panel_y + (5.0f * scale_y);

    for (const auto& msg : msg_history) {
        SDL_FRect dest_rect = { start_x, current_y, msg.width, msg.height };
        SDL_RenderTexture(renderer, msg.texture, nullptr, &dest_rect);
        current_y += msg.height + spacing;
    }

    if (active) {
        std::string prompt = "> " + player_input;

        if ((SDL_GetTicks() / 500) % 2 == 0) {
            prompt += "_";
        }

        float input_width = 0.0f;
        float input_height = 0.0f;
        SDL_Texture* input_texture = create_texture_from_msg(prompt, input_width, input_height);

        if (input_texture) {
            SDL_FRect dest_rect = { start_x, input_y, input_width, input_height };
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
