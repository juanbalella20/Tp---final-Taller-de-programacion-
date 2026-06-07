#include "screen_manager.h"

#include <stdexcept>
#include <utility>

#include "paths.h"
#include "welcome_screen.h"
#include "login_signup_screen.h"
#include "character_creation_screen.h"

ScreenManager::ScreenManager(std::string host, std::string port)
    : host(std::move(host)), port(std::move(port)) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::string("SDL_Init: ") + SDL_GetError());
    }
    window = SDL_CreateWindow(WINDOW_NAME, INITIAL_WIDTH, INITIAL_HEIGHT, 0);
    if (!window) {
        throw std::runtime_error(std::string("SDL_CreateWindow: ") + SDL_GetError());
    }
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        throw std::runtime_error(std::string("SDL_CreateRenderer: ") + SDL_GetError());
    }
    if (!TTF_Init()) {
        throw std::runtime_error(std::string("TTF_Init: ") + SDL_GetError());
    }
    const std::string font_path = paths::asset(FONT_PATH);
    font = TTF_OpenFont(font_path.c_str(), FONT_SIZE);
    if (!font) {
        throw std::runtime_error("No se pudo cargar " + font_path + ": " + SDL_GetError());
    }
}

ScreenManager::~ScreenManager() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    TTF_Quit();
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
}

std::unique_ptr<Screen> ScreenManager::make_screen(ScreenState state, AuthSession& session) {
    switch (state) {
        case ScreenState::WELCOME:
            return std::make_unique<WelcomeScreen>(renderer, window, font);
        case ScreenState::LOGIN_SIGNUP:
            return std::make_unique<LoginSignupScreen>(renderer, window, font, session, host, port);
        case ScreenState::CHARACTER_CREATION:
            return std::make_unique<CharacterCreationScreen>(renderer, window, font, session, host, port);
        default:
            // GAME / EXIT / CHARACTER_SELECTION (fuera de alcance) no tienen
            // Screen aca: el GAME lo arranca ClientApp.
            return nullptr;
    }
}

ScreenState ScreenManager::run(AuthSession& session) {
    ScreenState current = ScreenState::WELCOME;
    std::unique_ptr<Screen> screen = make_screen(current, session);

    SDL_Event event;
    while (screen) {
        while (SDL_PollEvent(&event)) {
            screen->handleEvent(event);
        }
        screen->update();

        ScreenState requested = screen->nextState();
        if (requested != current) {
            // Transicion: GAME/EXIT terminan el ciclo; el resto cambia de screen.
            if (requested == ScreenState::GAME || requested == ScreenState::EXIT) {
                return requested;
            }
            current = requested;
            screen = make_screen(current, session);
            continue;  // la nueva screen ya fijo su presentacion logica.
        }

        screen->render();
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    return ScreenState::EXIT;
}
