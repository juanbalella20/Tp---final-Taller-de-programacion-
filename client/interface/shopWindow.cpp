#include "shopWindow.h"

#include <SDL3_image/SDL_image.h>

#include <string>

#include "ui_helpers.h"
#include "paths.h"

// --- Layout (coordenadas del comercio.png de 1672x941) ----------------------
// Las regiones se convierten desde pixeles del arte al rect escalado del marco.
namespace {
constexpr float ART_W = 1672.0f;
constexpr float ART_H = 941.0f;

// La grilla visible tiene 6x9 slots. El borde del panel comienza en x=380,
// pero el primer slot comienza en x=438; usar el borde desplazaba todo una
// columna hacia la izquierda.
constexpr int GRID_COLS = 6;
constexpr int GRID_ROWS = 9;
constexpr float SLOT_X = 438.0f;
constexpr float SLOT_Y = 232.0f;
constexpr float SLOT_W = 56.0f;
constexpr float SLOT_H = 55.0f;
constexpr float SLOT_STEP_X = 58.0f;
constexpr float SLOT_STEP_Y = 58.0f;

// Zona util a la derecha del separador, sin incluir los margenes transparentes
// del PNG, y caja visible del boton CERRAR.
constexpr float DETAIL_X = 820.0f;
constexpr float DETAIL_Y = 139.0f;
constexpr float DETAIL_W = 462.0f;
constexpr float DETAIL_H = 708.0f;
constexpr float CLOSE_X = 721.0f;
constexpr float CLOSE_Y = 870.0f;
constexpr float CLOSE_W = 229.0f;
constexpr float CLOSE_H = 46.0f;
}  // namespace

ShopWindow::ShopWindow(SDL_Renderer* renderer, TTF_Font* font,
                       int logical_w, int logical_h)
    : renderer(renderer), font(font), texture_loader(renderer),
      logical_w(logical_w), logical_h(logical_h) {
    
    texture_loader.load_shop();
    texture_loader.load_items();
}

ShopWindow::~ShopWindow() {}

void ShopWindow::open(const std::vector<ItemInfo>& new_items) {
    items = new_items;
    selected_index = -1;
    open_ = true;
}

void ShopWindow::close() {
    open_ = false;
    selected_index = -1;
}

// El marco se dibuja centrado en el area logica, conservando el aspect ratio del
// PNG y ocupando ~94% de la altura disponible.
SDL_FRect ShopWindow::frame_rect() const {
    float h = logical_h * 0.94f;
    float w = h * (ART_W / ART_H);
    if (w > logical_w * 0.98f) {  // no exceder el ancho
        w = logical_w * 0.98f;
        h = w * (ART_H / ART_W);
    }
    float x = (logical_w - w) / 2.0f;
    float y = (logical_h - h) / 2.0f;
    return {x, y, w, h};
}

static SDL_FRect art_rect(const SDL_FRect& frame, float x, float y,
                          float w, float h) {
    return {frame.x + frame.w * (x / ART_W),
            frame.y + frame.h * (y / ART_H),
            frame.w * (w / ART_W),
            frame.h * (h / ART_H)};
}

SDL_FRect ShopWindow::grid_rect() const {
    const float width = SLOT_STEP_X * (GRID_COLS - 1) + SLOT_W;
    const float height = SLOT_STEP_Y * (GRID_ROWS - 1) + SLOT_H;
    return art_rect(frame_rect(), SLOT_X, SLOT_Y, width, height);
}
SDL_FRect ShopWindow::detail_rect() const {
    return art_rect(frame_rect(), DETAIL_X, DETAIL_Y, DETAIL_W, DETAIL_H);
}
SDL_FRect ShopWindow::close_rect() const {
    return art_rect(frame_rect(), CLOSE_X, CLOSE_Y, CLOSE_W, CLOSE_H);
}

SDL_FRect ShopWindow::slot_rect(int index) const {
    if (index < 0 || index >= GRID_COLS * GRID_ROWS) return {0, 0, 0, 0};
    int col = index % GRID_COLS;
    int row = index / GRID_COLS;
    return art_rect(frame_rect(),
                    SLOT_X + col * SLOT_STEP_X,
                    SLOT_Y + row * SLOT_STEP_Y,
                    SLOT_W, SLOT_H);
}

bool ShopWindow::handle_event(const SDL_Event& event) {
    if (!open_) return false;

    if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE) {
        close();
        return true;
    }
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        event.button.button == SDL_BUTTON_LEFT) {
        float lx, ly;
        SDL_RenderCoordinatesFromWindow(renderer, event.button.x, event.button.y,
                                        &lx, &ly);
        // Boton CERRAR.
        if (contains(close_rect(), lx, ly)) { close(); return true; }
        // Slot de item: selecciona el que este bajo el cursor.
        for (size_t i = 0; i < items.size(); ++i) {
            if (contains(slot_rect(static_cast<int>(i)), lx, ly)) {
                selected_index = static_cast<int>(i);
                return true;
            }
        }
        // Click fuera del marco: cerrar (como un modal). Click dentro del marco
        // pero fuera de slots/boton: consumir sin cerrar (no se filtra al juego).
        if (!contains(frame_rect(), lx, ly)) { close(); return true; }
        return true;
    }
    // Mientras esta abierta, consume todos los clicks y teclas de movimiento para
    // que no se filtren al juego (es modal). Deja pasar eventos no relevantes.
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
        event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        return true;
    }
    return false;
}

void ShopWindow::draw_text_centered(const std::string& text, const SDL_FRect& area,
                                    float y_offset, SDL_Color color) {
    if (text.empty() || !font) return;
    // Renderizamos el texto a una textura y la dibujamos centrada en 'area'. Si
    // el texto es mas ancho que el area (p.ej. "Precio: 350 monedas de oro"), lo
    // escalamos hacia abajo para que nunca se salga del panel.
    SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    float tw = static_cast<float>(surf->w);
    float th = static_cast<float>(surf->h);
    SDL_DestroySurface(surf);
    if (!tex) return;
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);

    const float max_w = area.w * 0.92f;  // margen interno del panel
    float scale = (tw > max_w) ? (max_w / tw) : 1.0f;
    float w = tw * scale;
    float h = th * scale;
    SDL_FRect dst = {area.x + (area.w - w) / 2.0f,
                     area.y + y_offset - h / 2.0f, w, h};
    SDL_RenderTexture(renderer, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
}

void ShopWindow::render() {
    if (!open_) return;

    SDL_FRect frame = frame_rect();

    // Atenuar el mundo detras (overlay semitransparente sobre toda el area).
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 150);
    SDL_FRect full = {0, 0, static_cast<float>(logical_w),
                      static_cast<float>(logical_h)};
    SDL_RenderFillRect(renderer, &full);

    // Marco de la tienda.
    SDL_Texture* background = texture_loader.get_shop_texture();
    if (background) {
        SDL_RenderTexture(renderer, background, nullptr, &frame);
    } else {
        // Fallback si falto el PNG: panel oscuro liso.
        SDL_SetRenderDrawColor(renderer, 30, 25, 20, 240);
        SDL_RenderFillRect(renderer, &frame);
    }

    const SDL_Color GOLD = {214, 178, 94, 255};
    const SDL_Color WHITE = {235, 235, 235, 255};
    const SDL_Color SEL = {255, 215, 0, 90};

    // Slots de la grilla con su item.
    for (size_t i = 0; i < items.size(); ++i) {
        int idx = static_cast<int>(i);
        SDL_FRect slot = slot_rect(idx);
        if (slot.w <= 0.0f) break;  // se acabaron los slots visibles

        // Resaltado del slot seleccionado.
        if (idx == selected_index) {
            SDL_FRect halo = {slot.x - 2.0f, slot.y /*- 2.0f*/,
                              slot.w /*+ 4.0f*/, slot.h /*+ 4.0f*/};
            SDL_SetRenderDrawColor(renderer, SEL.r, SEL.g, SEL.b, SEL.a);
            SDL_RenderFillRect(renderer, &halo);
        }

        // Sprite del item
        const ItemInfo& item = items[i];
        SDL_Texture* icon = texture_loader.get_texture_of_item(item.get_id());
        
        if (icon) {
            float tex_w = 0.0f;
            float tex_h = 0.0f;
            SDL_GetTextureSize(icon, &tex_w, &tex_h); 

            const float PAD = 4.0f;
            float max_w = slot.w - PAD * 2.0f;
            float max_h = slot.h - PAD * 2.0f;

            float scale = SDL_min(max_w / tex_w, max_h / tex_h);
            float fw = tex_w * scale;
            float fh = tex_h * scale;

            SDL_FRect dst = {
                slot.x + (slot.w - fw) / 2.0f,
                slot.y + (slot.h - fh) / 2.0f,
                fw,
                fh
            };
            SDL_RenderTexture(renderer, icon, nullptr, &dst);
        }
    }

    // Panel de detalle: nombre + precio del item seleccionado.
    SDL_FRect detail = detail_rect();
    if (selected_index >= 0 && selected_index < static_cast<int>(items.size())) {
        const ItemInfo& sel = items[selected_index];
        draw_text_centered(sel.get_name(), detail, detail.h * 0.38f, GOLD);
        std::string price = "Precio: " + std::to_string(sel.get_price()) +
                            " monedas de oro";
        draw_text_centered(price, detail, detail.h * 0.56f, WHITE);
    } else {
        draw_text_centered("Selecciona un objeto", detail, detail.h * 0.50f,
                           WHITE);
    }
}
