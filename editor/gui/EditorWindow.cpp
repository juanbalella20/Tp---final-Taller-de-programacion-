#include "EditorWindow.h"
#include <QAction>
#include <QActionGroup>
#include <QCollator>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QSignalBlocker>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QWidget>
#include <string>
#include <vector>

#include <algorithm>
#include <exception>

#include "../model/Map.h"
#include "../render/MapEditorScene.h"
#include "../render/TileLibrary.h"
#include "TilesetSelectorView.h"
#include "binaryMap/binaryMapSaver.h"
#include "protocol_constants.h" // Zone, ZONE_NAME_MAP_INV
#include "utility/paths.h"      // resource_relative / resolve_resource

EditorWindow::EditorWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Editor de Mapas - Argentum Online");
  resize(1024, 768);

  build_menus();
  build_workspace();
  build_tileset_toolbar();
  build_tools_toolbar();
}

void EditorWindow::build_workspace() {
  auto *splitter = new QSplitter(Qt::Horizontal, this);
  splitter->setChildrenCollapsible(false);

  selector_ = new TilesetSelectorView(splitter);
  selector_->setMinimumWidth(250);
  selector_->setMaximumWidth(750);

  map_scene_ = new MapEditorScene(&doc_, this);
  map_view_ = new QGraphicsView(map_scene_, splitter);
  map_view_->setAlignment(Qt::AlignLeft | Qt::AlignTop);
  map_view_->setDragMode(QGraphicsView::NoDrag);
  map_view_->setInteractive(true);
  map_view_->setAcceptDrops(true);
  map_view_->setBackgroundBrush(QColor(30, 30, 30));
  map_view_->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing,
                                 true);
  map_view_->setRenderHint(QPainter::Antialiasing, false);
  map_view_->setRenderHint(QPainter::SmoothPixmapTransform, false);
  map_view_->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

  splitter->addWidget(selector_);
  splitter->addWidget(map_view_);
  splitter->setCollapsible(0, false);
  splitter->setCollapsible(1, false);
  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);
  splitter->setSizes({250, 774});
  setCentralWidget(splitter);

  connect(selector_, &TilesetSelectorView::tileSelected, this,
          [this](int gid, const QPixmap &) {
            doc_.set_active_gid(gid);
            statusBar()->showMessage(
                QString("Tile seleccionado: gid %1").arg(gid));
          });
  connect(selector_, &TilesetSelectorView::brushSelected, this,
          [this](const TileBrush &brush) {
            statusBar()->showMessage(
                QString("Bloque seleccionado: %1 x %2 (%3 tiles)")
                    .arg(brush.width)
                    .arg(brush.height)
                    .arg(brush.gids.size()));
          });
}

void EditorWindow::build_tileset_toolbar() {
  auto *toolbar = addToolBar("Tilesets");
  toolbar->setObjectName("tilesetToolbar");
  toolbar->addWidget(new QLabel("Tileset: ", toolbar));

  tileset_combo_ = new QComboBox(toolbar);
  tileset_combo_->setMinimumContentsLength(16);
  toolbar->addWidget(tileset_combo_);
  // Todos los tilesets se pre-cargan desde assets/tilesets; para agregar uno
  // nuevo se deja el PNG en esa carpeta.
  load_all_tilesets();

  connect(tileset_combo_, &QComboBox::currentIndexChanged, this,
          &EditorWindow::select_tileset);
  connect(&doc_, &EditorDocument::tilesetsChanged, this,
          &EditorWindow::refresh_tileset_combo);
  refresh_tileset_combo();
}

void EditorWindow::load_all_tilesets() {
  const QString dir_path = QString::fromStdString(paths::tilesets_dir());
  QDir dir(dir_path);
  if (!dir.exists()) {
    statusBar()->showMessage(
        QString("No existe la carpeta de tilesets: %1").arg(dir_path));
    return;
  }

  const QStringList filters{"*.png", "*.PNG"};
  QFileInfoList entries = dir.entryInfoList(filters, QDir::Files, QDir::NoSort);
  QCollator collator;
  collator.setNumericMode(true);
  collator.setCaseSensitivity(Qt::CaseInsensitive);
  std::sort(entries.begin(), entries.end(),
            [&collator](const QFileInfo &a, const QFileInfo &b) {
              return collator.compare(a.fileName(), b.fileName()) < 0;
            });

  int loaded = 0;
  {
    // register_tileset_file() registra cada PNG en doc_; se bloquean sus
    // señales para no refrescar la UI por cada tileset, sino una sola vez al
    // final.
    QSignalBlocker blocker(&doc_);
    const auto &tilesets = doc_.map().tilesets();
    for (const QFileInfo &info : entries) {
      const std::string rel_path =
          paths::resource_relative(info.absoluteFilePath().toStdString());
      // devuelve true si ya hay un tileset cargado cuyo path
      // coincide con el archivo binario
      const bool already_loaded = std::any_of(
          tilesets.begin(), tilesets.end(),
          [&rel_path](const Tileset &ts) { return ts.file_path == rel_path; });
      if (already_loaded)
        continue;
      // Registra el nuevo tileset
      if (register_tileset_file(info.absoluteFilePath()) >= 0)
        ++loaded;
    }
  }
  doc_.notify_tilesets_changed();

  refresh_tileset_combo();
  if (loaded > 0) {
    tileset_combo_->setCurrentIndex(0);
    select_tileset(0);
  }

  statusBar()->showMessage(QString("Tilesets pre-cargados: %1 de %2")
                               .arg(loaded)
                               .arg(entries.size()));
}

int EditorWindow::register_tileset_file(const QString &path) {
  TileLibrary library;
  if (!library.loadTileset(path, doc_.map().tile_size())) {
    statusBar()->showMessage(
        QString("Tileset invalido o sin tiles completos: %1").arg(path));
    return -1;
  }

  const QFileInfo info(path);
  // El .bin guarda la ruta del tileset RELATIVA a la carpeta de recursos
  // compartida (ARGENTUM_RESOURCES_DIR).
  const std::string rel_path =
      paths::resource_relative(info.absoluteFilePath().toStdString());
  return doc_.register_tileset(info.baseName(),
                               QString::fromStdString(rel_path),
                               library.columns(), library.tileCount(), false);
}

void EditorWindow::refresh_tileset_combo() {
  if (!tileset_combo_)
    return;

  const int previousIndex = tileset_combo_->currentIndex();
  const QSignalBlocker blocker(tileset_combo_);
  tileset_combo_->clear();
  for (const Tileset &tileset : doc_.map().tilesets()) {
    tileset_combo_->addItem(QString::fromStdString(tileset.name));
  }

  if (tileset_combo_->count() == 0) {
    selector_->clearTileset();
    return;
  }
  tileset_combo_->setCurrentIndex(
      std::clamp(previousIndex, 0, tileset_combo_->count() - 1));
}

void EditorWindow::select_tileset(int index) {
  const auto &tilesets = doc_.map().tilesets();
  if (index < 0 || index >= static_cast<int>(tilesets.size()))
    return;

  const Tileset &tileset = tilesets[static_cast<std::size_t>(index)];
  // file_path es relativa a la carpeta de recursos: la resolvemos a absoluta
  // para poder abrir el PNG y mostrarlo en la paleta.
  const QString abs_path =
      QString::fromStdString(paths::resolve_resource(tileset.file_path));
  if (!selector_->setTileset(abs_path, doc_.map().tile_size(),
                             tileset.firstgid)) {
    selector_->clearTileset();
    statusBar()->showMessage(
        QString("No se pudo abrir el tileset: %1").arg(abs_path));
  }
}

void EditorWindow::build_tools_toolbar() {
  auto *toolbar = addToolBar("Herramientas");

  // Todas las herramientas (pintura + Teleport) son mutuamente exclusivas.
  auto *tools = new QActionGroup(this);
  tools->setExclusive(true);

  auto add_tool = [&](const QString &text, ToolType type, bool checked) {
    QAction *act = toolbar->addAction(text);
    act->setCheckable(true);
    act->setChecked(checked);
    tools->addAction(act);
    connect(act, &QAction::triggered, this,
            [this, type] { doc_.set_active_tool(type); });
    return act;
  };

  // --- Grupo 1: herramientas de pintura --------------------------------
  add_tool("Lapiz", ToolType::Pencil, true); // default
  add_tool("Goma", ToolType::Eraser, false);
  add_tool("Relleno", ToolType::Fill, false);

  toolbar->addSeparator();

  // --- Grupo 2: Teleport + Colision + capas de tiles -------------------
  QAction *teleport_tool = add_tool("Teleport", ToolType::Teleport, false);

  // Colision: pinta la grilla de bloqueo (rojo en el canvas). Es la fuente
  // unica de verdad de la colision del .bin, independiente de los graficos.
  add_tool("Colision", ToolType::Collision, false);

  // Combo de zona destino (label + combo en un solo widget). Define la zona a
  // la que apuntan las celdas marcadas con Teleport. Solo visible con esa
  // herramienta activa.
  auto *dest_widget = new QWidget(this);
  auto *dest_layout = new QHBoxLayout(dest_widget);
  dest_layout->setContentsMargins(4, 0, 4, 0);
  dest_layout->addWidget(new QLabel("Destino: "));
  auto *dest_combo = new QComboBox(dest_widget);
  for (Zone z :
       {ZONE_DESERT, ZONE_CITY, ZONE_FOREST, ZONE_TOWN, ZONE_DUNGEON}) {
    dest_combo->addItem(QString::fromStdString(ZONE_NAME_MAP_INV.at(z)));
  }
  dest_combo->setCurrentText(QString::fromStdString(doc_.active_dest_zone()));
  dest_layout->addWidget(dest_combo);
  dest_combo_action_ = toolbar->addWidget(dest_widget);
  dest_combo_action_->setVisible(false); // oculto hasta activar Teleport

  connect(dest_combo, &QComboBox::currentTextChanged, this,
          [this](const QString &zone) {
            doc_.set_active_dest_zone(zone.toStdString());
          });
  // Mostrar el combo solo cuando Teleport esta activa. setChecked(false) en una
  // accion del grupo exclusivo tambien dispara toggled, asi cubrimos ambos
  // lados.
  connect(teleport_tool, &QAction::toggled, this, [this](bool on) {
    if (dest_combo_action_)
      dest_combo_action_->setVisible(on);
  });

  // Capas de tiles editables (exclusivas entre si).
  auto *layers = new QActionGroup(this);
  layers->setExclusive(true);

  auto add_layer = [&](const QString &text, int idx, bool checked) {
    QAction *act = toolbar->addAction(text);
    act->setCheckable(true);
    act->setChecked(checked);
    layers->addAction(act);
    connect(act, &QAction::triggered, this,
            [this, idx] { doc_.set_active_layer(idx); });
  };
  add_layer("Suelo", Map::Ground, true); // default
  add_layer("Construcciones", Map::Buildings, false);
}

void EditorWindow::build_menus() {
  QMenu *archivo = menuBar()->addMenu("&Archivo");

  QAction *nuevo = archivo->addAction("&Nuevo");
  nuevo->setShortcut(QKeySequence::New);
  connect(nuevo, &QAction::triggered, this, &EditorWindow::on_new);

  QAction *abrir = archivo->addAction("&Abrir...");
  abrir->setShortcut(QKeySequence::Open);
  connect(abrir, &QAction::triggered, this, &EditorWindow::on_open);

  archivo->addSeparator();

  QAction *guardar = archivo->addAction("&Guardar");
  guardar->setShortcut(QKeySequence::Save);
  connect(guardar, &QAction::triggered, this, &EditorWindow::on_save);

  QAction *guardar_como = archivo->addAction("Guardar &como...");
  guardar_como->setShortcut(QKeySequence::SaveAs);
  connect(guardar_como, &QAction::triggered, this, &EditorWindow::on_save_as);

  archivo->addSeparator();

  QAction *salir = archivo->addAction("&Salir");
  salir->setShortcut(QKeySequence::Quit);
  connect(salir, &QAction::triggered, this, &QWidget::close);

  QAction *limpiar = archivo->addAction("&Limpiar");
  connect(limpiar, &QAction::triggered, this, &EditorWindow::on_clear);
}

void EditorWindow::on_new() {
  doc_.new_map();
  current_path_.clear();
  load_all_tilesets();
  statusBar()->showMessage("Nuevo mapa");
}

void EditorWindow::on_open() {
  const QString path = QFileDialog::getOpenFileName(
      this, "Abrir mapa", current_path_, "Mapa binario (*.bin)");
  if (path.isEmpty())
    return; // cancelado

  QString err;
  if (!doc_.open(path, &err)) {
    QMessageBox::warning(this, "Error al abrir",
                         QString("No se pudo abrir el mapa:\n%1").arg(err));
    statusBar()->showMessage("Error al abrir");
    return;
  }
  current_path_ = path;
  load_all_tilesets();
  statusBar()->showMessage(QString("Mapa abierto: %1").arg(path));
}
void EditorWindow::on_save() {
  // Sin ruta previa (mapa nunca guardado): comportarse como "Guardar como".
  if (current_path_.isEmpty()) {
    on_save_as();
    return;
  }
  save_to(current_path_);
}

void EditorWindow::on_save_as() {
  QString path = QFileDialog::getSaveFileName(
      this, "Guardar mapa", current_path_, "Mapa binario (*.bin)");
  if (path.isEmpty())
    return; // cancelado

  // Asegura la extension .bin si el usuario no la escribio.
  if (QFileInfo(path).suffix().isEmpty())
    path += ".bin";

  if (save_to(path))
    current_path_ = path;
}

void EditorWindow::on_clear() {
  doc_.clear_canvas();
}

bool EditorWindow::save_to(const QString &path) {
  const Map &map = doc_.map();
  std::vector<Tileset> tilesets = map.used_tilesets();
  for (Tileset &ts : tilesets) {
    ts.file_path = paths::resource_relative(ts.file_path);
  }

  try {
    BinaryMapSaver::save(path.toStdString(), map.tile_size(), map.width(),
                         map.height(), tilesets, map.layers(), map.teleports(),
                         map.collision());
  } catch (const std::exception &e) {
    QMessageBox::warning(
        this, "Error al guardar",
        QString("No se pudo guardar el mapa:\n%1").arg(e.what()));
    statusBar()->showMessage("Error al guardar");
    return false;
  }

  statusBar()->showMessage(QString("Mapa guardado: %1").arg(path));
  return true;
}
