#include "EditorDocument.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../tools/EraserTool.h"
#include "../tools/FillTool.h"
#include "../tools/PencilTool.h"
#include "SetCollisionCommand.h"
#include "SetTilesCommand.h"
#include "ToggleTeleportCommand.h"
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
  redo_stack_.clear();
  set_dirty(true);
  emit tilesetsChanged();
  emit undoStackChanged();
  return index;
}

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
    collision_changes_.clear();
    paint_collision(x, y);
    return;
  }
  // Resto: abre un gesto nuevo, fabrica la Tool y acumula sus primeros deltas.
  gesture_layer_ = active_layer_;
  gesture_tool_ = make_tool(tool);
  gesture_changes_.clear();
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
  // Colision: cierra el trazo y lo apila como UN solo SetCollisionCommand (sin
  // re-ejecutar: los cambios ya se aplicaron en vivo).
  if (collision_gesture_active_) {
    if (!collision_changes_.empty()) {
      undo_stack_.push_back(std::make_unique<SetCollisionCommand>(
          &map_, std::move(collision_changes_)));
      redo_stack_.clear();
      emit undoStackChanged();
    }
    collision_gesture_active_ = false;
    collision_changes_.clear();
    return;
  }
  // Cierra el gesto. Si toco al menos una celda, lo apila como UN solo
  // Command (sin re-ejecutar: los cambios ya se aplicaron en vivo).
  if (gesture_tool_ && !gesture_changes_.empty()) {
    push_committed_changes(gesture_layer_, std::move(gesture_changes_));
  }
  gesture_tool_.reset();
  gesture_changes_.clear();
}

void EditorDocument::paint_collision(int x, int y) {
  if (!map_.in_bounds(x, y))
    return;
  bool old_blocked = map_.is_blocked_cell(x, y);
  if (old_blocked == collision_paint_value_)
    // ya esta en ese estado
    return;
  map_.set_blocked(x, y, collision_paint_value_);
  collision_changes_.push_back({x, y, old_blocked, collision_paint_value_});
  set_dirty(true);
  emit cellChanged(-1, x, y); // layer -1: no es una capa de gids
}

void EditorDocument::toggle_teleport(int x, int y) {
  // Marca/desmarca (x,y) como teleport con la zona destino activa, apilando un
  // ToggleTeleportCommand para undo/redo. El canvas repinta via cellChanged.
  auto cmd =
      std::make_unique<ToggleTeleportCommand>(&map_, x, y, active_dest_zone_);
  cmd->execute();
  undo_stack_.push_back(std::move(cmd));
  redo_stack_.clear();
  set_dirty(true);
  emit cellChanged(-1, x, y); // layer -1: no es una capa de gids
  emit undoStackChanged();
}

void EditorDocument::apply_changes_live(std::vector<CellChange> changes) {
  // Aplica cada delta al Map y avisa al canvas; los acumula para el Command
  // que se arma al soltar.
  for (const CellChange &c : changes) {
    map_.set_cell(gesture_layer_, c.x, c.y, c.new_gid);
    emit cellChanged(gesture_layer_, c.x, c.y);
    gesture_changes_.push_back(c);
  }
  if (!changes.empty())
    set_dirty(true);
}

void EditorDocument::push_committed_changes(int layer,
                                            std::vector<CellChange> changes) {
  // Los cambios YA estan aplicados al Map; el Command se guarda solo para
  // poder revertirlos (undo) y reaplicarlos (redo). Un gesto nuevo invalida
  // el redo previo.
  undo_stack_.push_back(
      std::make_unique<SetTilesCommand>(&map_, layer, std::move(changes)));
  redo_stack_.clear();
  emit undoStackChanged();
}

// --- Undo / Redo -------------------------------------------------------------

void EditorDocument::undo() {
  if (undo_stack_.empty())
    return;
  std::unique_ptr<Command> cmd = std::move(undo_stack_.back());
  undo_stack_.pop_back();
  cmd->undo();
  redo_stack_.push_back(std::move(cmd));
  set_dirty(true);
  emit mapReset(); // el canvas no sabe que celdas cambio: repinta todo.
  emit undoStackChanged();
}

void EditorDocument::redo() {
  if (redo_stack_.empty())
    return;
  std::unique_ptr<Command> cmd = std::move(redo_stack_.back());
  redo_stack_.pop_back();
  cmd->execute();
  undo_stack_.push_back(std::move(cmd));
  set_dirty(true);
  emit mapReset();
  emit undoStackChanged();
}

bool EditorDocument::can_undo() const { return !undo_stack_.empty(); }
bool EditorDocument::can_redo() const { return !redo_stack_.empty(); }

// --- Persistencia ------------------------------------------------------------
// open() reconstruye el Map desde un .bin via BinaryMapLoader. El guardado lo
// hace EditorWindow::save_to (con BinaryMapSaver); save()/save_as() de aca son
// stubs que no se usan.

void EditorDocument::new_map() {
  // Vuelve al mapa vacio inicial y limpia todo el estado de edicion.
  map_ = Map();
  undo_stack_.clear();
  redo_stack_.clear();
  path_.clear();
  set_dirty(false);
  emit tilesetsChanged();
  emit mapReset();
  emit undoStackChanged();
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
  undo_stack_.clear();
  redo_stack_.clear();
  path_ = path;
  set_dirty(false);
  emit tilesetsChanged();
  emit mapReset();
  emit undoStackChanged();
  return true;
}

bool EditorDocument::save(const QString & /*path*/, QString *err) {
  // Stub sin usar: el guardado real vive en EditorWindow::save_to.
  if (err)
    *err = "Guardar mapa: no implementado todavia";
  return false;
}

bool EditorDocument::save_as(const QString &path, QString *err) {
  return save(path, err);
}

bool EditorDocument::is_dirty() const { return dirty_; }
QString EditorDocument::file_path() const { return path_; }

void EditorDocument::set_dirty(bool d) {
  if (d == dirty_)
    return;
  dirty_ = d;
  emit dirtyChanged(dirty_);
}
