#ifndef TILE_PALETTE_H
#define TILE_PALETTE_H

#include <QPixmap>
#include <QString>
#include <QWidget>
#include <vector>

#include "mapLoader.h"  // Tileset

namespace Ui {
class TilePalette;
}

class TilePalette : public QWidget {
    Q_OBJECT

public:
    explicit TilePalette(QWidget* parent = nullptr);
    ~TilePalette() override;

    const std::vector<Tileset>& tilesets() const { return tilesets_; }

signals:
    // gid del tile seleccionado (firstgid + local). 0 = ninguno.
    void tileSelected(int gid);

    // Emitida al cargar un PNG nuevo, para que el EditorWindow lo registre en el
    // Map (via add_tileset) y refresque el canvas. La paleta calcula el firstgid
    // con la misma formula que Map::next_firstgid y agrega en el mismo orden, asi
    // que los gid de la paleta y los del Map coinciden.
    void tilesetLoaded(const QString& name, const QString& file_path,
                       int columns, int tile_count, bool collidable);

private slots:
    void on_load_png();
    void on_item_selected();

private:
    Ui::TilePalette* ui;
    std::vector<Tileset> tilesets_;   // tilesets cargados
    std::vector<QPixmap> pixmaps_;    // QPixmap por tileset, alineado con tilesets_

    int next_firstgid() const;        // mayor (firstgid+tile_count), o 1
    // Corta el spritesheet en tiles y los agrega como items a la lista.
    void add_tiles_from(const Tileset& ts, const QPixmap& sheet);
};

#endif
