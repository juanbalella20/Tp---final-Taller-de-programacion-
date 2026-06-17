#ifndef EDITOR_GUI_TILEBRUSHMIME_H_
#define EDITOR_GUI_TILEBRUSHMIME_H_

#include <QByteArray>

#include "../document/TileBrush.h"
#define WxH_LIMIT 1000000

class QMimeData;

constexpr const char *TILE_BRUSH_MIME_TYPE =
    "application/x-argentum-tile-brush";

// Empaqueta un TileBrush en bytes para poder arrastrarlo.
QByteArray encode_tile_brush(const TileBrush &brush);

// Desempaqueta un TileBrush recibido por drag and drop.
// Devuelve true si el MIME era valido y escribe el resultado en *brush.
bool decode_tile_brush(const QMimeData *mime_data, TileBrush *brush);

#endif // EDITOR_GUI_TILEBRUSHMIME_H_
