#include "TeleportTool.h"

#include "../model/Map.h"

// Marca/desmarca la celda como teleport (toggle). Si esta vacia, escribe el
// marcador; si ya es teleport, la limpia. Devuelve {} si esta fuera del mapa.
std::vector<CellChange> TeleportTool::on_press(const Map& map, int layer,
                                               int x, int y, int /*active_gid*/) {
    if (!map.in_bounds(x, y)) return {};
    int old_gid = map.get_cell(layer, x, y);
    int new_gid = (old_gid == Map::TELEPORT_MARKER) ? 0 : Map::TELEPORT_MARKER;
    return {CellChange{x, y, old_gid, new_gid}};
}

std::vector<CellChange> TeleportTool::on_drag(const Map& /*map*/, int /*layer*/,
                                              int /*x*/, int /*y*/,
                                              int /*active_gid*/) {
    // Solo actua en el click: arrastrar no debe retogglear celdas.
    return {};
}

bool TeleportTool::paints_on_drag() const { return false; }
