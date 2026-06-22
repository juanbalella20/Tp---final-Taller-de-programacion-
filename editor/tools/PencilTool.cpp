#include "PencilTool.h"
#include <vector>

#include "../model/Map.h"

// Helper local: arma el CellChange de pintar 'gid' en (x,y) de 'layer'. 
// Devuelve {} si la celda esta fuera del mapa o ya tiene ese gid.
static std::vector<CellChange> paint_cell(const Map &map, int layer, int x,
                                          int y, int gid) {
  if (!map.in_bounds(x, y))
    return {};
  if (map.get_cell(layer, x, y) == gid)
    return {};
  return {CellChange{x, y, gid}};
}

std::vector<CellChange> PencilTool::on_press(const Map &map, int layer, int x,
                                             int y, int active_gid) {
  return paint_cell(map, layer, x, y, active_gid);
}

std::vector<CellChange> PencilTool::on_drag(const Map &map, int layer, int x,
                                            int y, int active_gid) {
  return paint_cell(map, layer, x, y, active_gid);
}

bool PencilTool::paints_on_drag() const { return true; }
