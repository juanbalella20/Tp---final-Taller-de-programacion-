#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

#include <QMainWindow>


class EditorWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit EditorWindow(QWidget* parent = nullptr);

private slots:
    void on_new();
    void on_open();
    void on_save();
    void on_save_as();

private:
    void build_menus();
};

#endif
