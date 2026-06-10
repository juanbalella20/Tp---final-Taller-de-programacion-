#ifndef EDITOR_DOCUMENT_COMMAND_H_
#define EDITOR_DOCUMENT_COMMAND_H_

/*
 * Interfaz del patron Command para el undo/redo. Cada operacion de edicion
 * reversible (un trazo de lapiz, una goma, un relleno) se encapsula en un
 * Command que sabe aplicarse y revertirse.
 *
 * El EditorDocument mantiene dos pilas de Command (undo/redo).
 *
 * Implementacion concreta: SetTilesCommand
 * (editor/document/SetTilesCommand.cpp).
 */
class Command {
public:
  virtual ~Command() = default;

  // Aplica el cambio al modelo.
  virtual void execute() = 0;
  // Revierte el cambio dejando el modelo como estaba antes de execute().
  virtual void undo() = 0;
};

#endif // EDITOR_DOCUMENT_COMMAND_H_
