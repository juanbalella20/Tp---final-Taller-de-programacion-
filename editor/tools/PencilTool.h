#ifndef EDITOR_PENCIL_TOOL_H
#define EDITOR_PENCIL_TOOL_H

#include "Tool.h"

/*
 * Lapiz: pinta el active_gid en la celda (x,y) de la capa activa. Pinta tanto
 * en el press como en el drag (trazo continuo). Devuelve un CellChange solo si
 * la celda cambia de valor (no genera deltas redundantes).
 *
 * 
 */
class PencilTool : public Tool {
public:
    std::vector<CellChange> on_press(const Map& map, int layer,
                                     int x, int y, int active_gid) override;
    std::vector<CellChange> on_drag(const Map& map, int layer,
                                    int x, int y, int active_gid) override;
    bool paints_on_drag() const override;
};

#endif  // EDITOR_PENCIL_TOOL_H
