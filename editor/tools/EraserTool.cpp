#include "EraserTool.h"

#include "../model/Map.h"

// La goma es un lapiz que siempre pinta gid 0 (celda vacia), ignorando el
// active_gid. Devuelve {} si la celda esta fuera del mapa o ya esta vacia.
static std::vector<CellChange> erase_cell(const Map& map, int layer, int x, int y) {
    if (!map.in_bounds(x, y)) return {};
    int old_gid = map.get_cell(layer, x, y);
    if (old_gid == 0) return {};
    return {CellChange{x, y, old_gid, 0}};
}

std::vector<CellChange> EraserTool::on_press(const Map& map, int layer,
                                             int x, int y, int /*active_gid*/) {
    return erase_cell(map, layer, x, y);
}

std::vector<CellChange> EraserTool::on_drag(const Map& map, int layer,
                                            int x, int y, int /*active_gid*/) {
    return erase_cell(map, layer, x, y);
}

bool EraserTool::paints_on_drag() const { return true; }
