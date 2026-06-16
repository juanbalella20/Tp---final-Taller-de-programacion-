#ifndef EDITOR_DOCUMENT_TILEBRUSH_H_
#define EDITOR_DOCUMENT_TILEBRUSH_H_

#include <vector>

// Bloque rectangular de gids, almacenados fila por fila
struct TileBrush {
  int width = 0;
  int height = 0;
  std::vector<int> gids;
};

#endif // EDITOR_DOCUMENT_TILEBRUSH_H_
