#ifndef MINICHAT_H_
#define MINICHAT_H_

#include <string>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <deque>
#include "../common/queue.h"

struct chatMessage {
    std::string text;
    SDL_Texture* texture;
    float width;
    float height;
}

class MiniChat {

private:
    bool is_active;
    std::string player_input;
    std::string outbound_msg;
    std::deque<chatMessage> msg_history;

    SDL_Renderer* renderer;
    TTF_Font* font;

    void pop_oldest_message();

public:
    MiniChat(SDL_Renderer* renderer, TTF_Font* font);

    bool handle_event(const SDL_Event& event);

    void toggle_active();

    SDL_Texture* create_texture_from_msg(const std::string& msg, float& width, float& height);

    bool has_pending_outbound_message() const;

    std::string pop_outbound_message();

    void update(Queue<std::string>& inbox);

    ~MiniChat();
};

#endif
