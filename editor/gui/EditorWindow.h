#ifndef EDITOR_GUI_EDITORWINDOW_H_
#define EDITOR_GUI_EDITORWINDOW_H_

#include <QMainWindow>
#include <QString>

#include "../document/EditorDocument.h"

class QAction;
class QComboBox;
class QGraphicsView;
class MapEditorScene;
class TilesetSelectorView;

/*
 * Ventana principal del editor. Arma los menus, las toolbars, la paleta y el
 * canvas, y cablea sus acciones al EditorDocument (dueno del Map). No tiene
 * logica de edicion: solo traduce la UI en llamadas al documento.
 */
class EditorWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit EditorWindow(QWidget *parent = nullptr);

private slots:
  void on_new();      // Archivo > Nuevo: descarta el mapa y arranca uno vacio.
  void on_open();     // Archivo > Abrir: carga un .bin elegido por el usuario.
  void on_save();     // Archivo > Guardar: reusa la ruta actual (o "Guardar como").
  void on_save_as();  // Archivo > Guardar como: pide ruta y serializa el .bin.
  void load_tileset(); // Pide un PNG por dialogo y lo agrega como tileset.

private:
  // Crea el menu Archivo (nuevo, abrir, guardar, guardar como, salir).
  void build_menus();
  // Crea el splitter central con la paleta y el canvas scrolleable.
  void build_workspace();
  // Crea la toolbar de tilesets (combo + boton "Cargar PNG").
  void build_tileset_toolbar();
  // Crea la toolbar de herramientas (pintura/teleport/colision) y de capas.
  void build_tools_toolbar();
  // Carga el PNG de 'path' como tileset y lo deja seleccionado. false si falla.
  bool load_tileset_path(const QString &path);
  // Repuebla el combo de tilesets desde el modelo.
  void refresh_tileset_combo();
  // Muestra en la paleta el tileset de indice 'index'.
  void select_tileset(int index);

  // Serializa el Map actual a 'path' (.bin via BinaryMapSaver). Muestra el
  // resultado en la status bar; devuelve true si se guardo bien.
  bool save_to(const QString &path);

  // Controlador: dueño del Map y del estado de edicion.
  EditorDocument doc_;
  TilesetSelectorView *selector_ = nullptr;
  MapEditorScene *map_scene_ = nullptr;
  QGraphicsView *map_view_ = nullptr;
  QComboBox *tileset_combo_ = nullptr;
  // Accion que envuelve el combo de zona destino (label + combo). Visible solo
  // cuando la herramienta activa es Teleport.
  QAction *dest_combo_action_ = nullptr;
  // Ruta del archivo actual (vacia si nunca se guardo). on_save reusa esta
  // ruta; si esta vacia delega en on_save_as.
  QString current_path_;
};

#endif // EDITOR_GUI_EDITORWINDOW_H_
