#include "FillTool.h"

#include <vector>

#include "../model/Map.h"

// Flood fill 4-conexo desde (x,y): cambia la region contigua con el mismo gid
// original por active_gid.
std::vector<CellChange> FillTool::on_press(const Map &map, int layer, int x,
                                           int y, int active_gid) {
  if (!map.in_bounds(x, y))
    return {};

  if (layer != Map::Ground)
    return {}; // solo se pinta layer de suelo

  int target = map.get_cell(layer, x, y);
  if (target == active_gid)
    return {}; // ya es del color destino: nada que rellenar

  const int w = map.width();
  const int h = map.height();

  std::vector<CellChange> changes;
  // Marca de celdas ya encoladas/visitadas para no repetirlas.
  std::vector<std::vector<bool>> visited(h, std::vector<bool>(w, false));

  // Pila explicita (evita recursion profunda en mapas grandes).
  std::vector<std::pair<int, int>> stack;
  stack.emplace_back(x, y);
  visited[y][x] = true;

  while (!stack.empty()) {
    auto [cx, cy] = stack.back();
    stack.pop_back();

    // Solo se expanden por celdas que tengan el gid original.
    if (map.get_cell(layer, cx, cy) != target)
      continue;

    changes.push_back(CellChange{cx, cy, active_gid});

    // Recorre los vecinos 4-conexos
    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};
    for (int i = 0; i < 4; ++i) {
      int nx = cx + dx[i];
      int ny = cy + dy[i];
      if (nx < 0 || ny < 0 || nx >= w || ny >= h)
        continue;
      if (visited[ny][nx])
        continue;
      if (map.get_cell(layer, nx, ny) != target)
        continue;
      visited[ny][nx] = true;
      stack.emplace_back(nx, ny);
    }
  }

  return changes;
}

std::vector<CellChange> FillTool::on_drag(const Map & /*map*/, int /*layer*/,
                                          int /*x*/, int /*y*/,
                                          int /*active_gid*/) {
  // El relleno actua solo en el click inicial.
  return {};
}

bool FillTool::paints_on_drag() const { return false; }
