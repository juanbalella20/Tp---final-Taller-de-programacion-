#ifndef SCREEN_H
#define SCREEN_H

#include <SDL3/SDL.h>

enum class ScreenState {
    WELCOME,
    LOGIN_SIGNUP,
    CHARACTER_SELECTION,
    CHARACTER_CREATION,
    GAME,
    EXIT
};

class Screen {
 public:
    virtual void handleEvent(const SDL_Event& event) = 0;
    virtual void update() = 0;
    virtual void render() = 0;
    virtual ScreenState nextState() const = 0;
    virtual ~Screen() = default;
};

#endif
