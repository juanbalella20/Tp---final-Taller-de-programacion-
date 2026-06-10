#ifndef EDITOR_SET_TILES_COMMAND_H
#define EDITOR_SET_TILES_COMMAND_H

#include <vector>

#include "CellChange.h"
#include "Command.h"

class Map;

/*
 * Command concreto: aplica/revierte un conjunto de cambios de celda sobre una
 * capa. Un trazo completo (press + drags) o un relleno se guardan como UN solo
 * SetTilesCommand, de modo que un gesto entero es una unica operacion de undo.
 *
 * Guarda solo las celdas tocadas (old/new gid), no un snapshot del mapa entero.
 *

 */
class SetTilesCommand : public Command {
public:
  SetTilesCommand(Map *map, int layer, std::vector<CellChange> changes);

  void execute() override; // setea new_gid en cada celda
  void undo() override;    // restaura old_gid en cada celda

private:
  Map *map_;
  int layer_;
  std::vector<CellChange> changes_;
};

#endif // EDITOR_SET_TILES_COMMAND_H
