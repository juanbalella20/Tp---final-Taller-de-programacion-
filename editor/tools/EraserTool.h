#ifndef EDITOR_ERASER_TOOL_H
#define EDITOR_ERASER_TOOL_H

#include "Tool.h"

/*
 * Goma: escribe gid 0 (celda vacia) en (x,y) de la capa activa. Es un lapiz que
 * siempre pinta 0, ignorando el active_gid. Pinta en press y en drag.
 *
 * 
 */
class EraserTool : public Tool {
public:
    std::vector<CellChange> on_press(const Map& map, int layer,
                                     int x, int y, int active_gid) override;
    std::vector<CellChange> on_drag(const Map& map, int layer,
                                    int x, int y, int active_gid) override;
    bool paints_on_drag() const override;
};

#endif  // EDITOR_ERASER_TOOL_H
