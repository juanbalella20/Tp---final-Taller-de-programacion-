#include "camera.h"

Camera::Camera(float viewport_w, float viewport_h)
    : x(0.0f), y(0.0f), w(viewport_w), h(viewport_h) {}

void Camera::center_on(float world_x, float world_y) {
    x = world_x - w / 2.0f;
    y = world_y - h / 2.0f;
}

void Camera::clamp_to(float map_pixel_w, float map_pixel_h) {
    if (x < 0.0f) x = 0.0f;
    if (y < 0.0f) y = 0.0f;
    if (x + w > map_pixel_w) x = map_pixel_w - w;
    if (y + h > map_pixel_h) y = map_pixel_h - h;
}
