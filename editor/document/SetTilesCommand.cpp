#include "SetTilesCommand.h"
#include <utility>
#include <vector>

#include "../model/Map.h"

SetTilesCommand::SetTilesCommand(Map *map, int layer,
                                 std::vector<CellChange> changes)
    : map_(map), layer_(layer), changes_(std::move(changes)) {}

void SetTilesCommand::execute() {
  // Setea new_gid en cada celda tocada.
  for (const CellChange &c : changes_) {
    map_->set_cell(layer_, c.x, c.y, c.new_gid);
  }
}

void SetTilesCommand::undo() {
  // Restaura old_gid, dejando el modelo como estaba antes de execute().
  for (const CellChange &c : changes_) {
    map_->set_cell(layer_, c.x, c.y, c.old_gid);
  }
}
