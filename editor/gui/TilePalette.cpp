#include "TilePalette.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QListWidget>
#include <QMessageBox>

#include "ui_TilePalette.h"  // generado por AUTOUIC desde TilePalette.ui
#include "game_constants.h"  // TILE_SIZE

TilePalette::TilePalette(QWidget* parent)
    : QWidget(parent), ui(new Ui::TilePalette) {
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::clicked, this, &TilePalette::load_png);
    connect(ui->listWidget, &QListWidget::itemSelectionChanged,
            this, &TilePalette::item_selected);
}

TilePalette::~TilePalette() { delete ui; }

int TilePalette::next_firstgid() const {
    int next = 1;  // gid 0 = vacio
    for (const auto& ts : tilesets_) {
        next = std::max(next, ts.firstgid + ts.tile_count);
    }
    return next;
}

void TilePalette::load_png() {
    QString path = QFileDialog::getOpenFileName(
        this, "Elegir imagen (PNG)", QString(), "Imagenes (*.png *.PNG)");
    if (path.isEmpty()) return;

    QPixmap sheet(path);
    if (sheet.isNull()) {
        QMessageBox::warning(this, "Error", "No se pudo cargar la imagen:\n" + path);
        return;
    }

    // Deriva columns/tile_count de las dimensiones reales del PNG (un tile por
    // celda de TILE_SIZE). El nombre del tileset es el del archivo.
    int cols = sheet.width() / TILE_SIZE;
    int rows = sheet.height() / TILE_SIZE;
    if (cols < 1) cols = 1;
    if (rows < 1) rows = 1;

    Tileset ts;
    ts.name = QFileInfo(path).baseName().toStdString();
    ts.file_path = path.toStdString();
    ts.columns = cols;
    ts.tile_count = cols * rows;
    ts.firstgid = next_firstgid();
    ts.collidable = false;

    tilesets_.push_back(ts);
    pixmaps_.push_back(sheet);
    add_tiles_from(ts, sheet);

    // Avisa al EditorWindow para que registre el mismo tileset en el Map (mismo
    // orden y misma formula de firstgid -> gids coinciden) y refresque el canvas.
    emit tilesetLoaded(QString::fromStdString(ts.name), path,
                       ts.columns, ts.tile_count, ts.collidable);
}

void TilePalette::add_tiles_from(const Tileset& ts, const QPixmap& sheet) {
    for (int local = 0; local < ts.tile_count; ++local) {
        int col = local % ts.columns;
        int row = local / ts.columns;
        QPixmap tile = sheet.copy(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE);

        int gid = ts.firstgid + local;
        auto* item = new QListWidgetItem(QIcon(tile), QString());
        item->setData(Qt::UserRole, gid);
        item->setToolTip(QString("%1 #%2 (gid %3)")
                             .arg(QString::fromStdString(ts.name))
                             .arg(local)
                             .arg(gid));
        ui->listWidget->addItem(item);
    }
}

void TilePalette::item_selected() {
    auto items = ui->listWidget->selectedItems();
    if (items.isEmpty()) {
        emit tileSelected(0);
        return;
    }
    emit tileSelected(items.first()->data(Qt::UserRole).toInt());
}
