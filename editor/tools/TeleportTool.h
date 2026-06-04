#ifndef TELEPORT_TOOL_H
#define TELEPORT_TOOL_H

#include "Tool.h"

/* Remarca celda de tipo ground para indicar que es apta teleport*/
class TeleportTool : public Tool {
public:
    std::vector<CellChange> on_press(const Map& map, int layer,
                                     int x, int y, int active_gid) override;
    std::vector<CellChange> on_drag(const Map& map, int layer,
                                    int x, int y, int active_gid) override;
    bool paints_on_drag() const override;
};

#endif  // TELEPORT_TOOL_H
