#ifndef EDITOR_CELL_CHANGE_H
#define EDITOR_CELL_CHANGE_H

/*
 * Delta de una celda: su gid anterior y el nuevo. Es la unidad de cambio que
 * producen las Tools y que guardan los Command (para undo/redo).
 *
 * Tipo compartido del contrato: lo usan Tool, SetTilesCommand y EditorDocument.
 */
struct CellChange {
    int x;
    int y;
    int old_gid;
    int new_gid;
};

#endif  // EDITOR_CELL_CHANGE_H
