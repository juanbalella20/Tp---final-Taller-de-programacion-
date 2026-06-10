#ifndef EDITOR_DOCUMENT_SETCOLLISIONCOMMAND_H_
#define EDITOR_DOCUMENT_SETCOLLISIONCOMMAND_H_

#include <vector>

#include "Command.h"

class Map;

/*
 * Command concreto: aplica/revierte un conjunto de cambios sobre la grilla de
 * colision del Map. Un trazo completo de la herramienta Colision (press +
 * drags) se guarda como UN solo SetCollisionCommand, igual que SetTilesCommand
 * para los gids: un gesto entero es una unica operacion de undo.
 *
 * La grilla de colision NO es una capa de gids: es una matriz booleana por
 * celda, por eso usa su propio delta (BlockedChange) en vez de CellChange.
 */
class SetCollisionCommand : public Command {
public:
  struct BlockedChange {
    int x;
    int y;
    bool old_blocked;
    bool new_blocked;
  };

  SetCollisionCommand(Map *map, std::vector<BlockedChange> changes);

  void execute() override; // setea new_blocked en cada celda
  void undo() override;    // restaura old_blocked en cada celda

private:
  Map *map_;
  std::vector<BlockedChange> changes_;
};

#endif // EDITOR_DOCUMENT_SETCOLLISIONCOMMAND_H_
