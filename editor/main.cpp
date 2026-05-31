#include <QApplication>

#include "gui/EditorWindow.h"

// Punto de entrada del editor grafico de mapas.
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    EditorWindow window;
    window.show();

    return app.exec();
}
