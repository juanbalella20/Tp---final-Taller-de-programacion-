#include "SetTilesCommand.h"

#include "../model/Map.h"

// ============================================================================
// Implementacion del Command de cambios de celda 
//
// Un gesto completo (trazo de lapiz, goma o relleno) se guarda como UN solo
// SetTilesCommand: execute() aplica los new_gid y undo() restaura los old_gid.
// Guarda solo las celdas tocadas, no un snapshot del mapa entero.
// ============================================================================

SetTilesCommand::SetTilesCommand(Map* map, int layer, std::vector<CellChange> changes)
    : map_(map), layer_(layer), changes_(std::move(changes)) {}

void SetTilesCommand::execute() {
    // Setea new_gid en cada celda tocada.
    for (const CellChange& c : changes_) {
        map_->set_cell(layer_, c.x, c.y, c.new_gid);
    }
}

void SetTilesCommand::undo() {
    // Restaura old_gid, dejando el modelo como estaba antes de execute().
    for (const CellChange& c : changes_) {
        map_->set_cell(layer_, c.x, c.y, c.old_gid);
    }
}
