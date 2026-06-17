#include "EditorDocument.h"
#include <memory>
#include <string>
#include <vector>

#include "../tools/EraserTool.h"
#include "../tools/FillTool.h"
#include "../tools/PencilTool.h"
#include "binaryMap/binaryMapLoader.h"

EditorDocument::EditorDocument(QObject *parent) : QObject(parent) {}

Map &EditorDocument::map() { return map_; }
const Map &EditorDocument::map() const { return map_; }

//  Estado de edicion

void EditorDocument::set_active_tool(ToolType t) { tool_ = t; }
ToolType EditorDocument::active_tool() const { return tool_; }

void EditorDocument::set_active_gid(int gid) { active_gid_ = gid; }
int EditorDocument::active_gid() const { return active_gid_; }

void EditorDocument::set_active_layer(int idx) {
  if (idx < 0 || idx >= map_.layer_count())
    return;
  if (idx == active_layer_)
    return;
  active_layer_ = idx;
  emit activeLayerChanged(active_layer_);
}
int EditorDocument::active_layer() const { return active_layer_; }

void EditorDocument::set_active_dest_zone(const std::string &zone) {
  active_dest_zone_ = zone;
}
const std::string &EditorDocument::active_dest_zone() const {
  return active_dest_zone_;
}

int EditorDocument::register_tileset(const QString &name,
                                     const QString &file_path, int columns,
                                     int tile_count, bool collidable) {
  const int index =
      map_.add_tileset(name.toStdString(), file_path.toStdString(), columns,
                       tile_count, collidable);
  set_dirty(true);
  emit tilesetsChanged();
  return index;
}

void EditorDocument::notify_tilesets_changed() { emit tilesetsChanged(); }

std::unique_ptr<Tool> EditorDocument::make_tool(ToolType t) const {
  switch (t) {
  case ToolType::Pencil:
    return std::make_unique<PencilTool>();
  case ToolType::Eraser:
    return std::make_unique<EraserTool>();
  case ToolType::Fill:
    return std::make_unique<FillTool>();
  // Teleport y Collision se desvian antes en begin_tool_gesture; nunca deberian
  // llegar aca. Devolvemos un lapiz por las dudas.
  case ToolType::Teleport:
  case ToolType::Collision:
    return std::make_unique<PencilTool>();
  }
  return std::make_unique<PencilTool>();
}

// --- Punto de entrada de las herramientas (lo llama el canvas) ---------------

void EditorDocument::apply_tool_press(int x, int y) {
  begin_tool_gesture(tool_, x, y);
}

void EditorDocument::apply_tool_press(int x, int y, ToolType tool_override) {
  begin_tool_gesture(tool_override, x, y);
}

void EditorDocument::begin_tool_gesture(ToolType tool, int x, int y) {
  // La herramienta Teleport no usa el flujo de gids: es un toggle de 1 celda
  // sobre el vector de teleports del Map (sin gesto press-drag-release).
  if (tool == ToolType::Teleport) {
    toggle_teleport(x, y);
    return;
  }
  // Colision: trazo de pintura sobre la grilla booleana. El valor a pintar se
  // fija ahora (el inverso de la celda inicial) y se mantiene durante el drag.
  if (tool == ToolType::Collision) {
    if (!map_.in_bounds(x, y))
      return;
    collision_gesture_active_ = true;
    collision_paint_value_ = !map_.is_blocked_cell(x, y);
    paint_collision(x, y);
    return;
  }
  // Resto: abre un gesto nuevo, fabrica la Tool y aplica cambios
  gesture_layer_ = active_layer_;
  gesture_tool_ = make_tool(tool);
  apply_changes_live(
      gesture_tool_->on_press(map_, gesture_layer_, x, y, active_gid_));
}

void EditorDocument::apply_tool_drag(int x, int y) {
  // Colision: continua el trazo con el valor fijado en el press.
  if (collision_gesture_active_) {
    paint_collision(x, y);
    return;
  }
  // Solo las herramientas que pintan al arrastrar (lapiz/goma) actuan aca.
  if (!gesture_tool_ || !gesture_tool_->paints_on_drag())
    return;
  apply_changes_live(
      gesture_tool_->on_drag(map_, gesture_layer_, x, y, active_gid_));
}

void EditorDocument::apply_tool_release(int /*x*/, int /*y*/) {
  // Los cambios ya se aplicaron al Map en vivo durante press/drag: el release
  // solo cierra el gesto en curso.
  collision_gesture_active_ = false;
  gesture_tool_.reset();
}

void EditorDocument::stamp_tiles(int start_x, int start_y,
                                 const TileBrush &brush) {
  if (brush.width <= 0 || brush.height <= 0 ||
      brush.gids.size() != static_cast<std::size_t>(brush.width) *
                               static_cast<std::size_t>(brush.height)) {
    return;
  }

  bool changed = false;
  for (int row = 0; row < brush.height; ++row) {
    for (int column = 0; column < brush.width; ++column) {
      const int map_x = start_x + column;
      const int map_y = start_y + row;
      // Si una parte del bloque cae fuera, se saltea.
      if (!map_.in_bounds(map_x, map_y))
        continue;

      const int gid =
          brush.gids[static_cast<std::size_t>(row * brush.width + column)];
      // Si la celda ya tiene ese tile, se saltea
      if (map_.get_cell(active_layer_, map_x, map_y) == gid)
        continue;

      map_.set_cell(active_layer_, map_x, map_y, gid);
      emit cellChanged(active_layer_, map_x, map_y);
      changed = true;
    }
  }

  if (changed)
    set_dirty(true);
}

void EditorDocument::paint_collision(int x, int y) {
  if (!map_.in_bounds(x, y))
    return;
  bool old_blocked = map_.is_blocked_cell(x, y);
  if (old_blocked == collision_paint_value_)
    // ya esta en ese estado
    return;
  map_.set_blocked(x, y, collision_paint_value_);
  set_dirty(true);
  emit cellChanged(-1, x, y); // layer -1: no es una capa de gids
}

void EditorDocument::toggle_teleport(int x, int y) {
  // Marca/desmarca (x,y) como teleport con la zona destino activa, directo
  // sobre el Map. El canvas repinta via cellChanged.
  if (map_.teleport_at(x, y)) {
    map_.remove_teleport(x, y);
  } else {
    map_.add_teleport(x, y, active_dest_zone_);
  }
  set_dirty(true);
  emit cellChanged(-1, x, y); // layer -1: no es una capa de gids
}

void EditorDocument::apply_changes_live(std::vector<CellChange> changes) {
  // Aplica cada cambio al Map en vivo y avisa al canvas para que repinte.
  for (const CellChange &c : changes) {
    map_.set_cell(gesture_layer_, c.x, c.y, c.new_gid);
    emit cellChanged(gesture_layer_, c.x, c.y);
  }
  if (!changes.empty())
    set_dirty(true);
}

// --- Persistencia ------------------------------------------------------------
// open() reconstruye el Map desde un .bin via BinaryMapLoader. El guardado lo
// hace EditorWindow::save_to (con BinaryMapSaver); save()/save_as() de aca son
// stubs que no se usan.

void EditorDocument::new_map() {
  // Vuelve al mapa vacio inicial y limpia todo el estado de edicion.
  map_ = Map();
  path_.clear();
  set_dirty(false);
  emit tilesetsChanged();
  emit mapReset();
}

void EditorDocument::clear_canvas() {
  map_.clear_canvas();
  set_dirty(true);
  emit mapReset();
}

bool EditorDocument::open(const QString &path, QString *err) {
  try {
    BinaryMapLoader loader;
    loader.load(path.toStdString());
    map_.load_from(loader.get_tilesets(), loader.get_layers(),
                   loader.get_collision(), loader.get_teleports());
  } catch (const std::exception &e) {
    if (err)
      *err = e.what();
    return false;
  }
  path_ = path;
  set_dirty(false);
  emit tilesetsChanged();
  emit mapReset();
  return true;
}

bool EditorDocument::is_dirty() const { return dirty_; }
QString EditorDocument::file_path() const { return path_; }

void EditorDocument::set_dirty(bool d) {
  if (d == dirty_)
    return;
  dirty_ = d;
  emit dirtyChanged(dirty_);
}
