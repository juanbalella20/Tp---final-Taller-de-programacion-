#ifndef LOAD_TILESET_DIALOG_H
#define LOAD_TILESET_DIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class LoadTilesetDialog;
}

class LoadTilesetDialog : public QDialog {
    Q_OBJECT

public:
    // 'image_path' es el PNG ya elegido; se usa para sugerir valores y validar.
    explicit LoadTilesetDialog(const QString& image_path, QWidget* parent = nullptr);
    ~LoadTilesetDialog() override;

    QString tileset_name() const;
    int columns() const;
    int tile_count() const;
    bool collidable() const;

private:
    Ui::LoadTilesetDialog* ui;
    QString image_path_;
};

#endif  // LOAD_TILESET_DIALOG_H
