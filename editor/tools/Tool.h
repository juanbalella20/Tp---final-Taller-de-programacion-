#ifndef EDITOR_TOOLS_TOOL_H_
#define EDITOR_TOOLS_TOOL_H_

#include <vector>

#include "../model/Map.h"

struct CellChange {
  int x;
  int y;
  int gid;
};

class Tool {
public:
  virtual ~Tool() = default;

  // Devuelve los cambios a aplicar (vacio si no hay nada que cambiar). 
  // 'active_gid' es el tile seleccionado.
  virtual std::vector<CellChange> on_press(const Map &map, int layer, int x,
                                           int y, int active_gid) = 0;

  // Continuacion del gesto (drag con boton apretado). Para Fill devuelve {}.
  virtual std::vector<CellChange> on_drag(const Map &map, int layer, int x,
                                          int y, int active_gid) = 0;

  // True si la herramienta pinta tambien al arrastrar (Pencil/Eraser).
  // False si solo actua en el press (Fill).
  virtual bool paints_on_drag() const = 0;
};

#endif // EDITOR_TOOLS_TOOL_H_
