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

WelcomeScreen::~WelcomeScreen() {
    freeMedia();
    freeSDL();
}

void WelcomeScreen::initSDL() {
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::string("SDL_InitSubSystem: ") + SDL_GetError());
    }
    video_initialized = true;

    window = SDL_CreateWindow(WINDOW_NAME, LAUNCHER_WIDTH, LAUNCHER_HEIGHT, 0);
    if (!window) {
        throw std::runtime_error(std::string("SDL_CreateWindow: ") + SDL_GetError());
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        throw std::runtime_error(std::string("SDL_CreateRenderer: ") + SDL_GetError());
    }

    SDL_SetRenderLogicalPresentation(
        renderer, LAUNCHER_WIDTH, LAUNCHER_HEIGHT,
        SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

void WelcomeScreen::freeSDL() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    if (video_initialized) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        video_initialized = false;
    }
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

    if (!TTF_Init()) {
        throw std::runtime_error(std::string("TTF_Init: ") + SDL_GetError());
    }
    ttf_initialized = true;

    const std::string font_path = paths::asset(FONT_PATH);
    config_font = TTF_OpenFont(font_path.c_str(), 11);
    if (!config_font) {
        throw std::runtime_error(
            "No se pudo cargar " + font_path + ": " + SDL_GetError());
    }
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
    if (config_font) {
        TTF_CloseFont(config_font);
        config_font = nullptr;
    }
    if (ttf_initialized) {
        TTF_Quit();
        ttf_initialized = false;
    }
}

void WelcomeScreen::handleEvent(const SDL_Event& event, bool& is_running) {
    if (showing_config) {
        handleConfigEvent(event, is_running);
        return;
    }

    handleMainEvent(event, is_running);
}

void WelcomeScreen::handleMainEvent(const SDL_Event& event, bool& is_running) {
    switch (event.type) {
        case SDL_EVENT_QUIT:
            is_running = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.scancode == SDL_SCANCODE_RETURN ||
                event.key.scancode == SDL_SCANCODE_SPACE) {
                result.start_game = true;
                is_running = false;
            } else if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                is_running = false;
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
                is_running = false;
            } else if (contains(CLOSE_BUTTON, x, y)) {
                is_running = false;
            } else if (contains(SETTINGS_BUTTON, x, y)) {
                showConfigScreen();
            }
            break;
        }
        default:
            break;
    }
}

void WelcomeScreen::handleConfigEvent(const SDL_Event& event, bool& is_running) {
    switch (event.type) {
        case SDL_EVENT_QUIT:
            is_running = false;
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
            } else if (contains(RESOLUTION_BUTTON, config_x, config_y)) {
                resolution_dropdown_open = !resolution_dropdown_open;
            } else if (resolution_dropdown_open) {
                const int option = resolutionOptionAt(config_x, config_y);
                if (option >= 0) {
                    setResolution(option);
                } else {
                    resolution_dropdown_open = false;
                }
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

    SDL_RenderPresent(renderer);
}

void WelcomeScreen::showMainScreen() {
    showing_config = false;
    resolution_dropdown_open = false;
}

void WelcomeScreen::showConfigScreen() {
    pending_settings = result.settings;
    pending_resolution_index = selected_resolution_index;
    showing_config = true;
    resolution_dropdown_open = false;
}

void WelcomeScreen::set_fullscreen(bool fs) {
    pending_settings.fullscreen = fs;
}

void WelcomeScreen::setResolution(int index) {
    pending_resolution_index = index;
    resolution_dropdown_open = false;

    switch (index) {
        case 0:
            pending_settings.width = 1280;
            pending_settings.height = 720;
            break;
        case 1:
            pending_settings.width = 1366;
            pending_settings.height = 768;
            break;
        case 2:
            pending_settings.width = 1600;
            pending_settings.height = 900;
            break;
        case 3:
            pending_settings.width = 1920;
            pending_settings.height = 1080;
            break;
        default:
            break;
    }
}

void WelcomeScreen::applySettings() {
    result.settings = pending_settings;
    selected_resolution_index = pending_resolution_index;
    showMainScreen();
}

void WelcomeScreen::discardSettings() {
    pending_settings = result.settings;
    pending_resolution_index = selected_resolution_index;
    showMainScreen();
}

SDL_FRect WelcomeScreen::resolutionOptionRect(int index) const {
    return SDL_FRect{
        RESOLUTION_BUTTON.x,
        RESOLUTION_BUTTON.y + RESOLUTION_BUTTON.h +
            index * RESOLUTION_OPTION_HEIGHT,
        RESOLUTION_BUTTON.w,
        RESOLUTION_OPTION_HEIGHT
    };
}

int WelcomeScreen::resolutionOptionAt(float config_x, float config_y) const {
    for (int i = 0; i < RESOLUTION_OPTION_COUNT; ++i) {
        if (contains(resolutionOptionRect(i), config_x, config_y)) {
            return i;
        }
    }
    return -1;
}

const char* WelcomeScreen::resolutionLabel(int index) const {
    switch (index) {
        case 0:
            return "1280 x 720";
        case 1:
            return "1366 x 768";
        case 2:
            return "1600 x 900";
        case 3:
            return "1920 x 1080";
        default:
            return "1280 x 720";
    }
}

void WelcomeScreen::drawConfigControls() {
    SDL_FRect resolution_rect = toPopupRect(RESOLUTION_BUTTON);
    drawText(
        resolutionLabel(pending_resolution_index),
        resolution_rect.x + 18.0f,
        resolution_rect.y + 9.0f,
        SDL_Color{255, 255, 255, 255});

    if (pending_settings.fullscreen) {
        drawTick(FULLSCREEN_BUTTON);
    } else {
        drawTick(WINDOWED_BUTTON);
    }

    if (resolution_dropdown_open) {
        drawResolutionDropdown();
    }
}

void WelcomeScreen::drawResolutionDropdown() {
    for (int i = 0; i < RESOLUTION_OPTION_COUNT; ++i) {
        SDL_FRect option = toPopupRect(resolutionOptionRect(i));
        const bool selected = (i == pending_resolution_index);

        SDL_SetRenderDrawColor(renderer, 10, 10, 10, 235);
        SDL_RenderFillRect(renderer, &option);
        SDL_SetRenderDrawColor(
            renderer,
            selected ? 40 : 130,
            selected ? 210 : 130,
            selected ? 70 : 130,
            255);
        SDL_RenderRect(renderer, &option);

        drawText(
            resolutionLabel(i),
            option.x + 18.0f,
            option.y + 5.0f,
            selected ? SDL_Color{80, 255, 110, 255}
                     : SDL_Color{255, 255, 255, 255});
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

void WelcomeScreen::drawText(const char* text, float x, float y, SDL_Color color) {
    if (!config_font) {
        return;
    }

    SDL_Surface* surface = TTF_RenderText_Blended(config_font, text, 0, color);
    if (!surface) {
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_DestroySurface(surface);
        return;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_FRect dst{
        x,
        y,
        static_cast<float>(surface->w),
        static_cast<float>(surface->h)
    };
    SDL_DestroySurface(surface);
    SDL_RenderTexture(renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

LauncherResult WelcomeScreen::run() {
    result = LauncherResult{};
    pending_settings = result.settings;
    selected_resolution_index = 0;
    pending_resolution_index = 0;
    resolution_dropdown_open = false;
    showing_config = false;

    try {
        initSDL();
        loadMedia();

        bool is_running = true;
        while (is_running) {
            while (SDL_PollEvent(&event)) {
                handleEvent(event, is_running);
            }

            render();
            SDL_Delay(16);
        }
    } catch (...) {
        freeMedia();
        freeSDL();
        throw;
    }

    freeMedia();
    freeSDL();
    return result;
}
