#ifndef EDITOR_FILL_TOOL_H
#define EDITOR_FILL_TOOL_H

#include "Tool.h"

/*
 * Relleno (flood fill 4-conexo): desde (x,y), reemplaza la region contigua de
 * celdas que comparten el gid original por el active_gid, en la capa activa.
 * Solo actua en el press; on_drag devuelve {} y paints_on_drag() es false.
 *
 *
 */
class FillTool : public Tool {
public:
  std::vector<CellChange> on_press(const Map &map, int layer, int x, int y,
                                   int active_gid) override;
  std::vector<CellChange> on_drag(const Map &map, int layer, int x, int y,
                                  int active_gid) override;
  bool paints_on_drag() const override;
};

#endif // EDITOR_FILL_TOOL_H
