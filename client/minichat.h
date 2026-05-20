#ifndef MINICHAT_H_
#define MINICHAT_H_

#include <string>
#include <SDL3/SDL.h>
#include <deque>
#include "../common/queue.h"

class MiniChat {

private:
    bool is_active;
    std::string player_input;
    std::string outbound_msg;
    std::deque<std::string> msg_history;

    SDL_Renderer* renderer;

public:
    MiniChat(SDL_Renderer* renderer);

    bool is_active() const;

    bool handle_event(const SDL_Event& event);

    bool has_pending_outbound_message() const;

    std::string pop_outbound_message();

    void update(Queue<std::string>& inbox);
};

#endif
