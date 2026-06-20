#ifndef SHOP_WINDOW_H
#define SHOP_WINDOW_H

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <map>
#include <string>
#include <vector>

#include "item_info.h"

// Ventana modal de comercio (la "tienda" del vendedor). Se abre con /listar al
// lado de un comerciante: muestra el catalogo recibido en MSG_LIST.
//
// Interaccion (solo lectura, no compra):
//   - Panel izquierdo: grilla de slots con el sprite de cada item del stock.
//   - Al clickear un slot: el item queda seleccionado y a la derecha se muestra
//     su nombre y su precio.
//   - Boton CERRAR (o tecla ESC / click fuera del marco): cierra la ventana.
//
// El fondo es imagenes/comercio.png escalado al area logica; las regiones
// (grilla / panel de detalle / boton) se posicionan como fracciones de ese
// marco para mantenerse alineadas con el arte sin medir pixeles exactos.
class ShopWindow {
public:
    // renderer/font son COMPARTIDOS (propiedad del ScreenManager): no se liberan
    // aca. logical_w/logical_h son las dimensiones del area de presentacion
    // logica (las mismas que usa el resto de la GUI).
    ShopWindow(SDL_Renderer* renderer, TTF_Font* font,
               int logical_w, int logical_h);
    ~ShopWindow();

    ShopWindow(const ShopWindow&) = delete;
    ShopWindow& operator=(const ShopWindow&) = delete;

    // Abre la ventana con el catalogo dado (resetea la seleccion).
    void open(const std::vector<ItemInfo>& items);
    // Cierra la ventana.
    void close();

    // Procesa un evento SDL. Devuelve true si lo consumio (la ventana esta
    // abierta y el evento le corresponde): en ese caso el resto de la GUI debe
    // ignorarlo (la tienda es modal). Maneja: click en slot (selecciona), click
    // en CERRAR / fuera del marco (cierra), ESC (cierra).
    bool handle_event(const SDL_Event& event);

    // Dibuja la ventana. No hace nada si esta cerrada. Llamar al final del frame
    // (sobre el HUD), con el viewport completo restaurado.
    void render();

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    int logical_w;
    int logical_h;

    bool open_ = false;
    std::vector<ItemInfo> items;
    int selected_index = -1;  // -1 = ningun item seleccionado

    SDL_Texture* background = nullptr;  // comercio.png
    // Sprites de los items, por id (cargados una vez en el ctor). Misma tabla
    // que el HUD: el icono de la tienda se ve igual que el del inventario.
    std::map<std::string, SDL_Texture*> item_textures;
    std::map<std::string, SDL_FRect> item_crops;

    // Rect del marco comercio.png en pantalla (centrado en el area logica),
    // recalculado cada render por si cambia el tamaño logico.
    SDL_FRect frame_rect() const;
    // Rects de las regiones, derivados del marco (fracciones del PNG).
    SDL_FRect grid_rect() const;    // panel izquierdo: grilla de items
    SDL_FRect detail_rect() const;  // panel derecho: nombre + precio
    SDL_FRect close_rect() const;   // boton CERRAR

    // Geometria de la grilla: rect del slot 'index' dentro de grid_rect().
    // columns/rows/slot fijos; devuelve {0,0,0,0} si el index no entra.
    SDL_FRect slot_rect(int index) const;

    void load_item_textures();
    void draw_text_centered(const std::string& text, const SDL_FRect& area,
                            float y_offset, SDL_Color color);
};

#endif  // SHOP_WINDOW_H
