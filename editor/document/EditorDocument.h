#ifndef EDITOR_DOCUMENT_H
#define EDITOR_DOCUMENT_H

#include <memory>
#include <vector>

#include <QObject>
#include <QString>

#include "../model/Map.h"
#include "../tools/ToolType.h"
#include "CellChange.h"
#include "Command.h"
#include "Tool.h"

/*
 * Controlador del editor. Ata el modelo (Map) con el estado de edicion
 * (herramienta/brush/capa activos), el undo/redo y la persistencia.
 *
 * Es el UNICO que muta el Map: los widgets (canvas, paleta, panel de capas)
 * solo le mandan intenciones y leen su estado. No hay logica de negocio en los
 * widgets. Hereda de QObject solo para emitir senales que la GUI escucha.
 *
 * Hace de puente con la persistencia (common/): al guardar extrae los datos del
 * Map y se los pasa a BinaryMapSaver; al abrir reconstruye el Map desde
 * BinaryMapLoader. BinaryMapSaver/Loader NO conocen al Map.
 *
 * Implementacion: Persona B (editor/document/EditorDocument.cpp). C++20.
 */
class EditorDocument : public QObject {
    Q_OBJECT

public:
    explicit EditorDocument(QObject* parent = nullptr);

    // Acceso de lectura al modelo para los widgets de render/paleta.
    Map& map();
    const Map& map() const;

    // --- Estado de edicion (flujo paleta -> canvas -> modelo) -------------
    void set_active_tool(ToolType t);
    ToolType active_tool() const;
    void set_active_gid(int gid);   // brush seleccionado en la paleta
    int active_gid() const;
    void set_active_layer(int idx);
    int active_layer() const;

    // --- Punto de entrada de las herramientas (lo llama el canvas) --------
    // Aplican la herramienta activa en (x,y). Internamente acumulan CellChange
    // y, al soltar, arman un unico SetTilesCommand y lo apilan en el undo stack.
    void apply_tool_press(int x, int y);
    void apply_tool_drag(int x, int y);
    void apply_tool_release(int x, int y);

    // --- Undo / Redo ------------------------------------------------------
    void undo();
    void redo();
    bool can_undo() const;
    bool can_redo() const;

    // --- Persistencia (delega en common/ BinaryMapSaver/Loader) -----------
    // V1: el mapa es de tamano fijo (WIDTH x HEIGHT), asi que "Nuevo" no recibe
    // dimensiones: crea un Map vacio (dos capas en 0).
    void new_map();
    bool open(const QString& path, QString* err);
    bool save(const QString& path, QString* err);
    bool save_as(const QString& path, QString* err);

    bool is_dirty() const;
    QString file_path() const;

signals:
    // Tras new/open: el canvas reconstruye todo (sceneRect, repintado completo).
    void mapReset();
    // Una celda cambio: el canvas invalida solo ese rect.
    void cellChanged(int layer, int x, int y);
    void dirtyChanged(bool dirty);
    void activeLayerChanged(int idx);
    // Las pilas de undo/redo cambiaron: la GUI actualiza el enable de sus acciones.
    void undoStackChanged();

private:
    Map map_;
    ToolType tool_ = ToolType::Pencil;
    int active_gid_ = 0;
    int active_layer_ = 0;
    bool dirty_ = false;
    QString path_;
    std::vector<std::unique_ptr<Command>> undo_stack_;
    std::vector<std::unique_ptr<Command>> redo_stack_;

    // Estado del gesto en curso (entre press y release). gesture_tool_ es la
    // Tool fabricada en el press; gesture_changes_ acumula todos los CellChange
    // del gesto para armar UN solo SetTilesCommand al soltar.
    std::unique_ptr<Tool> gesture_tool_;
    std::vector<CellChange> gesture_changes_;

    // Capa sobre la que actua el gesto actual: Teleports si la herramienta
    // activa es Teleport (capa oculta), o la capa de tiles activa en otro caso.
    int effective_layer() const;
    // Fabrica la Tool concreta segun tool_.
    std::unique_ptr<Tool> make_tool(ToolType t) const;
    // Aplica los deltas al Map en vivo, emite cellChanged y los acumula en
    // gesture_changes_ (para el Command que se arma al soltar).
    void apply_changes_live(std::vector<CellChange> changes);
    // Apila los cambios (ya aplicados) como un SetTilesCommand para undo/redo.
    void push_committed_changes(int layer, std::vector<CellChange> changes);
    void set_dirty(bool d);
};

#endif  // EDITOR_DOCUMENT_H
