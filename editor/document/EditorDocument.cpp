#include "EditorDocument.h"

#include "../tools/EraserTool.h"
#include "../tools/FillTool.h"
#include "../tools/PencilTool.h"
#include "../tools/TeleportTool.h"
#include "SetTilesCommand.h"

EditorDocument::EditorDocument(QObject* parent) : QObject(parent) {}

Map& EditorDocument::map() { return map_; }
const Map& EditorDocument::map() const { return map_; }

// --- Estado de edicion -------------------------------------------------------

void EditorDocument::set_active_tool(ToolType t) { tool_ = t; }
ToolType EditorDocument::active_tool() const { return tool_; }

void EditorDocument::set_active_gid(int gid) { active_gid_ = gid; }
int EditorDocument::active_gid() const { return active_gid_; }

void EditorDocument::set_active_layer(int idx) {
    if (idx < 0 || idx >= map_.layer_count()) return;
    if (idx == active_layer_) return;
    active_layer_ = idx;
    emit activeLayerChanged(active_layer_);
}
int EditorDocument::active_layer() const { return active_layer_; }

int EditorDocument::effective_layer() const {
    // La herramienta Teleport edita siempre la capa Teleports (oculta), sin
    // importar que capa de tiles este seleccionada en el toolbar.
    return tool_ == ToolType::Teleport ? Map::Teleports : active_layer_;
}

std::unique_ptr<Tool> EditorDocument::make_tool(ToolType t) const {
    switch (t) {
        case ToolType::Pencil: return std::make_unique<PencilTool>();
        case ToolType::Eraser: return std::make_unique<EraserTool>();
        case ToolType::Fill:   return std::make_unique<FillTool>();
        case ToolType::Teleport: return std::make_unique<TeleportTool>();
    }
    return std::make_unique<PencilTool>();  // fallback defensivo
}

// --- Punto de entrada de las herramientas (lo llama el canvas) ---------------

void EditorDocument::apply_tool_press(int x, int y) {
    // Abre un gesto nuevo: fabrica la Tool activa y acumula sus primeros deltas.
    gesture_tool_ = make_tool(tool_);
    gesture_changes_.clear();
    apply_changes_live(gesture_tool_->on_press(map_, effective_layer(), x, y,
                                               active_gid_));
}

void EditorDocument::apply_tool_drag(int x, int y) {
    // Solo las herramientas que pintan al arrastrar (lapiz/goma) actuan aca.
    if (!gesture_tool_ || !gesture_tool_->paints_on_drag()) return;
    apply_changes_live(gesture_tool_->on_drag(map_, effective_layer(), x, y,
                                              active_gid_));
}

void EditorDocument::apply_tool_release(int /*x*/, int /*y*/) {
    // Cierra el gesto. Si toco al menos una celda, lo apila como UN solo
    // Command (sin re-ejecutar: los cambios ya se aplicaron en vivo).
    if (gesture_tool_ && !gesture_changes_.empty()) {
        push_committed_changes(effective_layer(), std::move(gesture_changes_));
    }
    gesture_tool_.reset();
    gesture_changes_.clear();
}

void EditorDocument::apply_changes_live(std::vector<CellChange> changes) {
    // Aplica cada delta al Map y avisa al canvas; los acumula para el Command
    // que se arma al soltar.
    const int layer = effective_layer();
    for (const CellChange& c : changes) {
        map_.set_cell(layer, c.x, c.y, c.new_gid);
        emit cellChanged(layer, c.x, c.y);
        gesture_changes_.push_back(c);
    }
    if (!changes.empty()) set_dirty(true);
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
    if (undo_stack_.empty()) return;
    std::unique_ptr<Command> cmd = std::move(undo_stack_.back());
    undo_stack_.pop_back();
    cmd->undo();
    redo_stack_.push_back(std::move(cmd));
    set_dirty(true);
    emit mapReset();  // el canvas no sabe que celdas cambio: repinta todo.
    emit undoStackChanged();
}

void EditorDocument::redo() {
    if (redo_stack_.empty()) return;
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

// --- Persistencia (STUB: pendiente de integrar BinaryMapLoader/Saver) --------

void EditorDocument::new_map() {
    // TODO(persistencia): reemplazar map_ por un Map vacio y limpiar el estado.
    map_ = Map();
    undo_stack_.clear();
    redo_stack_.clear();
    path_.clear();
    set_dirty(false);
    emit mapReset();
    emit undoStackChanged();
}

bool EditorDocument::open(const QString& /*path*/, QString* err) {
    // TODO(persistencia): cargar con BinaryMapLoader (common/) y reconstruir map_.
    if (err) *err = "Abrir mapa: no implementado todavia";
    return false;
}

bool EditorDocument::save(const QString& /*path*/, QString* err) {
    // TODO(persistencia): extraer los datos de map_ y delegar en BinaryMapSaver.
    if (err) *err = "Guardar mapa: no implementado todavia";
    return false;
}

bool EditorDocument::save_as(const QString& path, QString* err) {
    return save(path, err);
}

bool EditorDocument::is_dirty() const { return dirty_; }
QString EditorDocument::file_path() const { return path_; }

void EditorDocument::set_dirty(bool d) {
    if (d == dirty_) return;
    dirty_ = d;
    emit dirtyChanged(dirty_);
}
