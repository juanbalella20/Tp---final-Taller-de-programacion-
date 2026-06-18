#ifndef CAMERA_H
#define CAMERA_H

#include <SDL3/SDL.h>

class Camera {
    float x;
    float y;
    float w;
    float h;

public:
    Camera(float viewport_w, float viewport_h);

    // Centra el viewport en (world_x, world_y) en pixeles del mundo.
    void center_on(float world_x, float world_y);

    // Limita la posicion del viewport a los bordes del mapa (pixeles).
    void clamp_to(float map_pixel_w, float map_pixel_h);

    // pantalla = mundo - origen del viewport
    float world_to_screen_x(float world_x) const { return world_x - x; }
    float world_to_screen_y(float world_y) const { return world_y - y; }

    // mundo = pantalla + origen del viewport
    float screen_to_world_x(float screen_x) const { return screen_x + x; }
    float screen_to_world_y(float screen_y) const { return screen_y + y; }

    float get_x() const { return x; }
    float get_y() const { return y; }
    float get_w() const { return w; }
    float get_h() const { return h; }
};

#endif
