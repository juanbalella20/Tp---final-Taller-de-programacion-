#include "ToggleTeleportCommand.h"

#include "../model/Map.h"

ToggleTeleportCommand::ToggleTeleportCommand(Map* map, int x, int y,
                                             std::string dest_zone)
    : map_(map), x_(x), y_(y), dest_zone_(std::move(dest_zone)),
      was_present_(false) {
    // Captura el estado previo de la celda para poder revertir el toggle.
    const TeleportDef* existing = map_->teleport_at(x_, y_);
    if (existing) {
        was_present_ = true;
        prev_zone_ = existing->dest_zone;
    }
}

void ToggleTeleportCommand::execute() {
    // Toggle: si la celda ya era teleport, la limpia; si no, la marca con la
    // zona destino activa.
    if (was_present_) {
        map_->remove_teleport(x_, y_);
    } else {
        map_->add_teleport(x_, y_, dest_zone_);
    }
}

void ToggleTeleportCommand::undo() {
    // Restaura exactamente el estado previo: si habia teleport, lo repone con su
    // dest_zone original; si no, lo quita.
    if (was_present_) {
        map_->add_teleport(x_, y_, prev_zone_);
    } else {
        map_->remove_teleport(x_, y_);
    }
}
