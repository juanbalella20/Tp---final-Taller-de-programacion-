#ifndef EDITOR_DOCUMENT_EDITORDOCUMENT_H_
#define EDITOR_DOCUMENT_EDITORDOCUMENT_H_

#include <memory>
#include <string>
#include <vector>

#include <QObject>
#include <QString>

#include "../model/Map.h"
#include "../tools/ToolType.h"
#include "CellChange.h"
#include "Tool.h"

/*
 * Controlador del editor. Ata el modelo (Map) con el estado de edicion
 * (herramienta/brush/capa activos) y la persistencia.
 *
 * Es el UNICO que muta el Map: los widgets (canvas, paleta, panel de capas)
 * solo le mandan intenciones y leen su estado. No hay logica de negocio en los
 * widgets. Hereda de QObject solo para emitir senales que la GUI escucha.
 *
 * Lado de carga de la persistencia (common/): open() reconstruye el Map desde
 * un .bin con BinaryMapLoader. El guardado, en cambio, lo hace
 * EditorWindow::save_to con BinaryMapSaver (save()/save_as() de aca son stubs).
 *
 */
class EditorDocument : public QObject {
  Q_OBJECT

public:
  explicit EditorDocument(QObject *parent = nullptr);

  // Acceso de lectura al modelo para los widgets de render/paleta.
  Map &map();
  const Map &map() const;

  // Estado de edicion (la herramienta, el brush y la capa que usan los gestos)
  void set_active_tool(ToolType t); // herramienta activa de la toolbar
  ToolType active_tool() const;
  void set_active_gid(int gid); // brush seleccionado en la paleta
  int active_gid() const;
  void set_active_layer(int idx); // capa activa; emite activeLayerChanged
  int active_layer() const;
  // Zona destino activa para la herramienta Teleport (nombre: "city", etc.).
  void set_active_dest_zone(const std::string &zone);
  const std::string &active_dest_zone() const;

  // Registra un tileset en el modelo y avisa a las vistas para que
  // reconstruyan sus caches de pixmaps.
  int register_tileset(const QString &name, const QString &file_path,
                       int columns, int tile_count, bool collidable);
  // Emite tilesetsChanged() una sola vez. Lo usa la carga masiva inicial, que
  // registra varios tilesets y avisa a las vistas una unica vez.
  void notify_tilesets_changed();

  // Punto de entrada de las herramientas
  // Aplican la herramienta activa en (x,y), mutando el Map en vivo.
  void apply_tool_press(int x, int y);
  // Gesto temporal sin cambiar la herramienta seleccionada en la toolbar.
  void apply_tool_press(int x, int y, ToolType tool_override);
  void apply_tool_drag(int x, int y);
  void apply_tool_release(int x, int y);

  // Persistencia
  void new_map();
  bool open(const QString &path, QString *err); // via BinaryMapLoader

  bool is_dirty() const;     // hay cambios sin guardar
  QString file_path() const; // ruta del .bin abierto (vacia si es mapa nuevo)

signals:
  // Tras new/open: el canvas reconstruye todo (sceneRect, repintado completo).
  void mapReset();
  // Una celda cambio: el canvas invalida solo ese rect.
  void cellChanged(int layer, int x, int y);
  void dirtyChanged(bool dirty);
  void activeLayerChanged(int idx);
  void tilesetsChanged();

private:
  Map map_;
  ToolType tool_ = ToolType::Pencil;
  int active_gid_ = 0;
  int active_layer_ = 0;
  std::string active_dest_zone_ = "city"; // zona destino default de Teleport
  bool dirty_ = false;
  QString path_;

  // Estado del gesto en curso (entre press y release). gesture_tool_ es la
  // Tool fabricada en el press; gesture_layer_ es la capa sobre la que pinta.
  std::unique_ptr<Tool> gesture_tool_;
  int gesture_layer_ = 0;

  // Estado del gesto de la herramienta Colision. Es un trazo de pintura sobre
  // la grilla booleana (no gids): el valor a pintar se fija en el press (el
  // inverso del estado de la celda inicial) y se aplica en press+drag.
  bool collision_gesture_active_ = false;
  bool collision_paint_value_ = true; // valor que pinta el trazo en curso

  // Marca/desmarca (x,y) como teleport con la zona destino activa. Lo llama
  // apply_tool_press cuando la herramienta activa es Teleport (no pasa por el
  // flujo de gids).
  void toggle_teleport(int x, int y);
  // Pinta una celda de la grilla de colision dentro del gesto en curso, con el
  // valor fijado en el press.
  void paint_collision(int x, int y);
  // Fabrica la Tool concreta segun tool_.
  std::unique_ptr<Tool> make_tool(ToolType t) const;
  void begin_tool_gesture(ToolType tool, int x, int y);
  // Aplica los cambios al Map en vivo y emite cellChanged por cada celda.
  void apply_changes_live(std::vector<CellChange> changes);
  void set_dirty(bool d);
};

#endif // EDITOR_DOCUMENT_EDITORDOCUMENT_H_
