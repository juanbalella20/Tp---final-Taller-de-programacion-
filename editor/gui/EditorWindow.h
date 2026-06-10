#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

#include <QMainWindow>
#include <QString>

#include "../document/EditorDocument.h"

class QAction;
class QComboBox;
class QGraphicsView;
class MapEditorScene;
class TilesetSelectorView;

class EditorWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit EditorWindow(QWidget *parent = nullptr);

private slots:
  void on_new();
  void on_open();
  void on_save();
  void on_save_as();
  void load_tileset();

private:
  // Crea menu: archivo,nuevo,abrir,guardar, guardar como, salir
  void build_menus();
  // Crea el splitter central con la paleta y el canvas scrolleable.
  void build_workspace();
  void build_tileset_toolbar();
  // Crea la toolbar: herramientas (lapiz/goma/relleno) y capa activa.
  void build_tools_toolbar();
  // Crea menu Editar (deshacer/rehacer) y guarda las acciones.
  void build_edit_menu();
  // Habilita/deshabilita deshacer/rehacer segun el estado del document.
  void update_undo_actions();
  bool load_tileset_path(const QString &path);
  void refresh_tileset_combo();
  void select_tileset(int index);

  // Serializa el Map actual a 'path' (.bin via BinaryMapSaver). Muestra el
  // resultado en la status bar; devuelve true si se guardo bien.
  bool save_to(const QString &path);

  // Controlador: dueno del Map, del estado de edicion y del undo/redo.
  EditorDocument doc_;
  TilesetSelectorView *selector_ = nullptr;
  MapEditorScene *map_scene_ = nullptr;
  QGraphicsView *map_view_ = nullptr;
  QComboBox *tileset_combo_ = nullptr;
  QAction *undo_action_ = nullptr;
  QAction *redo_action_ = nullptr;
  // Accion que envuelve el combo de zona destino (label + combo). Visible solo
  // cuando la herramienta activa es Teleport.
  QAction *dest_combo_action_ = nullptr;
  // Ruta del archivo actual (vacia si nunca se guardo). on_save reusa esta
  // ruta; si esta vacia delega en on_save_as.
  QString current_path_;
};

#endif
