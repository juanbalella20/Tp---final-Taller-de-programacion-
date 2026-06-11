#ifndef EDITOR_TOOLS_TOOL_H_
#define EDITOR_TOOLS_TOOL_H_

#include <vector>

#include "../document/CellChange.h"

#include "../model/Map.h"

/*
 * Estrategia de edicion. Una Tool NO muta el Map directamente: dado el estado
 * del mapa y una celda, devuelve los CellChange a aplicar sobre la capa activa.
 * El EditorDocument es quien los aplica al Map.
 *
 * Implementaciones: PencilTool, EraserTool, FillTool (editor/tools/*.cpp).
 *
 */
class Tool {
public:
  virtual ~Tool() = default;

  // Inicio de un gesto (click). Devuelve los cambios a aplicar (puede ser
  // vacio si no hay nada que cambiar). 'active_gid' es el brush seleccionado.
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
