#ifndef WORLD_ENTITY_H
#define WORLD_ENTITY_H

#include "camera.h"

// Interfaz comun para cualquier cosa que vive en coordenadas del mundo
// (player, NPCs, items , oro) y se dibuja con offset de camara.
class WorldEntity {
public:
    virtual ~WorldEntity() = default;
    virtual void draw(const Camera& camera) const = 0;
};

#endif
