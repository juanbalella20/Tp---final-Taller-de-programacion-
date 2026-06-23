#include "welcome_screen.h"

#include "paths.h"

#include <SDL3_image/SDL_image.h>
#include <stdexcept>
#include <string>

bool WelcomeScreen::contains(const SDL_FRect& rect, float x, float y) {
    return x >= rect.x && x <= rect.x + rect.w &&
           y >= rect.y && y <= rect.y + rect.h;
}

bool WelcomeScreen::toConfigCoordinates(float x, float y, float& config_x, float& config_y) {
    if (!contains(CONFIG_POPUP, x, y)) {
        return false;
    }

    config_x = (x - CONFIG_POPUP.x) * CONFIG_IMAGE_WIDTH / CONFIG_POPUP.w;
    config_y = (y - CONFIG_POPUP.y) * CONFIG_IMAGE_HEIGHT / CONFIG_POPUP.h;
    return true;
}

SDL_FRect WelcomeScreen::toPopupRect(const SDL_FRect& config_rect) {
    const float scale_x = CONFIG_POPUP.w / CONFIG_IMAGE_WIDTH;
    const float scale_y = CONFIG_POPUP.h / CONFIG_IMAGE_HEIGHT;

    return SDL_FRect{
        CONFIG_POPUP.x + config_rect.x * scale_x,
        CONFIG_POPUP.y + config_rect.y * scale_y,
        config_rect.w * scale_x,
        config_rect.h * scale_y
    };
}

WelcomeScreen::WelcomeScreen(SDL_Renderer* renderer, SDL_Window* window, TTF_Font* /*font*/)
    : window(window), renderer(renderer) {
    // No creamos el window/renderer: son del ScreenManager. Solo fijamos la
    // presentacion logica de esta pantalla y cargamos sus texturas.
    SDL_SetRenderLogicalPresentation(
        renderer, LAUNCHER_WIDTH, LAUNCHER_HEIGHT,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);

    result = LauncherResult{};
    pending_settings = result.settings;

    loadMedia();
}

WelcomeScreen::~WelcomeScreen() {
    freeMedia();
}

void WelcomeScreen::loadMedia() {
    const std::string bground_path = paths::asset(BACKGROUND_PATH);
    SDL_Surface* main_surface = SDL_LoadBMP(bground_path.c_str());
    if (!main_surface) {
        throw std::runtime_error(
            "No se pudo cargar " + bground_path + ": " + SDL_GetError());
    }
    background = SDL_CreateTextureFromSurface(renderer, main_surface);
    SDL_DestroySurface(main_surface);
    if (!background) {
        throw std::runtime_error(
            std::string("SDL_CreateTextureFromSurface: ") + SDL_GetError());
    }

    const std::string config_path = paths::asset(CONFIG_PATH);
    SDL_Surface* config_surface = IMG_Load(config_path.c_str());
    if (!config_surface) {
        throw std::runtime_error(
            "No se pudo cargar " + config_path + ": " + SDL_GetError());
    }

    config = SDL_CreateTextureFromSurface(renderer, config_surface);
    SDL_DestroySurface(config_surface);
    if (!config) {
        throw std::runtime_error(
            std::string("SDL_CreateTextureFromSurface: ") + SDL_GetError());
    }

    media_loaded = true;
}

void WelcomeScreen::freeMedia() {
    if (background) {
        SDL_DestroyTexture(background);
        background = nullptr;
    }
    if (config) {
        SDL_DestroyTexture(config);
        config = nullptr;
    }
    media_loaded = false;
}

void WelcomeScreen::handleEvent(const SDL_Event& event) {
    if (showing_config) {
        handleConfigEvent(event);
        return;
    }

    handleMainEvent(event);
}

void WelcomeScreen::update() {}

void WelcomeScreen::handleMainEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_QUIT:
            next = ScreenState::EXIT;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.scancode == SDL_SCANCODE_RETURN ||
                event.key.scancode == SDL_SCANCODE_SPACE) {
                result.start_game = true;
                next = ScreenState::LOGIN_SIGNUP;
            } else if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                next = ScreenState::EXIT;
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if (event.button.button != SDL_BUTTON_LEFT) {
                break;
            }

            float x = 0.0f;
            float y = 0.0f;
            SDL_RenderCoordinatesFromWindow(
                renderer, event.button.x, event.button.y, &x, &y);

            if (contains(START_BUTTON, x, y)) {
                result.start_game = true;
                next = ScreenState::LOGIN_SIGNUP;
            } else if (contains(CLOSE_BUTTON, x, y)) {
                next = ScreenState::EXIT;
            } else if (contains(SETTINGS_BUTTON, x, y)) {
                showConfigScreen();
            }
            break;
        }
        default:
            break;
    }
}

void WelcomeScreen::handleConfigEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_QUIT:
            next = ScreenState::EXIT;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                showMainScreen();
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if (event.button.button != SDL_BUTTON_LEFT) {
                break;
            }

            float x = 0.0f;
            float y = 0.0f;
            SDL_RenderCoordinatesFromWindow(
                renderer, event.button.x, event.button.y, &x, &y);

            float config_x = 0.0f;
            float config_y = 0.0f;
            if (!toConfigCoordinates(x, y, config_x, config_y)) {
                break;
            }

            if (contains(CONFIG_CLOSE_BUTTON, config_x, config_y) ||
                contains(CONFIG_CANCEL_BUTTON, config_x, config_y)) {
                discardSettings();
            } else if (contains(WINDOWED_BUTTON, config_x, config_y)) {
                set_fullscreen(false);
            } else if (contains(FULLSCREEN_BUTTON, config_x, config_y)) {
                set_fullscreen(true);
            } else if (contains(CONFIG_APPLY_BUTTON, config_x, config_y)) {
                applySettings();
            }
            break;
        }
        default:
            break;
    }
}

void WelcomeScreen::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, background, nullptr, nullptr);

    if (showing_config) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
        SDL_FRect overlay{0.0f, 0.0f, LAUNCHER_WIDTH, LAUNCHER_HEIGHT};
        SDL_RenderFillRect(renderer, &overlay);
        SDL_RenderTexture(renderer, config, nullptr, &CONFIG_POPUP);
        drawConfigControls();
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
    // El SDL_RenderPresent lo hace el ScreenManager (dueño del frame).
}

ScreenState WelcomeScreen::nextState() const {
    return next;
}

void WelcomeScreen::showMainScreen() {
    showing_config = false;
}

void WelcomeScreen::showConfigScreen() {
    pending_settings = result.settings;
    showing_config = true;
}

void WelcomeScreen::set_fullscreen(bool fs) {
    pending_settings.fullscreen = fs;
}

void WelcomeScreen::applySettings() {
    result.settings = pending_settings;
    showMainScreen();
}

void WelcomeScreen::discardSettings() {
    pending_settings = result.settings;
    showMainScreen();
}

void WelcomeScreen::drawConfigControls() {
    if (pending_settings.fullscreen) {
        drawTick(FULLSCREEN_BUTTON);
    } else {
        drawTick(WINDOWED_BUTTON);
    }
}

void WelcomeScreen::drawTick(const SDL_FRect& config_rect) {
    SDL_FRect rect = toPopupRect(config_rect);
    const float x1 = rect.x + rect.w * 0.22f;
    const float y1 = rect.y + rect.h * 0.52f;
    const float x2 = rect.x + rect.w * 0.42f;
    const float y2 = rect.y + rect.h * 0.72f;
    const float x3 = rect.x + rect.w * 0.80f;
    const float y3 = rect.y + rect.h * 0.25f;

    SDL_SetRenderDrawColor(renderer, 70, 255, 90, 255);
    for (float offset = -1.0f; offset <= 1.0f; offset += 1.0f) {
        SDL_RenderLine(renderer, x1, y1 + offset, x2, y2 + offset);
        SDL_RenderLine(renderer, x2, y2 + offset, x3, y3 + offset);
    }
}
