#include "LoadTilesetDialog.h"

#include <QFileInfo>
#include <QImageReader>

#include "ui_LoadTilesetDialog.h"  // generado por AUTOUIC desde LoadTilesetDialog.ui
#include "game_constants.h"        // TILE_SIZE

LoadTilesetDialog::LoadTilesetDialog(const QString& image_path, QWidget* parent)
    : QDialog(parent), ui(new Ui::LoadTilesetDialog), image_path_(image_path) {
    ui->setupUi(this);

    // Dimensiones reales del PNG para sugerir columns/tile_count y mostrar info.
    QImageReader reader(image_path);
    QSize size = reader.size();
    int cols_guess = size.isValid() ? size.width() / TILE_SIZE : 1;
    int rows_guess = size.isValid() ? size.height() / TILE_SIZE : 1;
    if (cols_guess < 1) cols_guess = 1;
    if (rows_guess < 1) rows_guess = 1;

    ui->imagenLabel->setText(QFileInfo(image_path).fileName());
    ui->dimsLabel->setText(
        size.isValid()
            ? QString("%1 x %2 px (tile %3)").arg(size.width()).arg(size.height()).arg(TILE_SIZE)
            : QString("desconocido"));
    ui->columnsSpin->setValue(cols_guess);
    ui->tileCountSpin->setValue(cols_guess * rows_guess);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

LoadTilesetDialog::~LoadTilesetDialog() { delete ui; }

QString LoadTilesetDialog::tileset_name() const {
    // El .ui no tiene campo de nombre: usamos el nombre base del archivo.
    return QFileInfo(image_path_).baseName();
}

int LoadTilesetDialog::columns() const { return ui->columnsSpin->value(); }
int LoadTilesetDialog::tile_count() const { return ui->tileCountSpin->value(); }
bool LoadTilesetDialog::collidable() const { return ui->collidableCheck->isChecked(); }
