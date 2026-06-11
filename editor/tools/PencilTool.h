#ifndef EDITOR_TOOLS_PENCILTOOL_H_
#define EDITOR_TOOLS_PENCILTOOL_H_

#include "Tool.h"
#include <vector>

/*
 * Lapiz: pinta el active_gid en la celda (x,y) de la capa activa. Pinta tanto
 * en el press como en el drag (trazo continuo). Devuelve un CellChange solo si
 * la celda cambia de valor (no genera deltas redundantes).
 */
class PencilTool : public Tool {
public:
  std::vector<CellChange> on_press(const Map &map, int layer, int x, int y,
                                   int active_gid) override;
  std::vector<CellChange> on_drag(const Map &map, int layer, int x, int y,
                                  int active_gid) override;
  bool paints_on_drag() const override;
};

#endif // EDITOR_TOOLS_PENCILTOOL_H_
