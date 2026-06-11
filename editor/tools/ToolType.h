#ifndef EDITOR_TOOLS_TOOLTYPE_H_
#define EDITOR_TOOLS_TOOLTYPE_H_

/*
 * Herramientas de edicion disponibles. La GUI (toolbar) selecciona una y la
 * comunica al EditorDocument, que fabrica la Tool concreta correspondiente.
 *
 * Extensible: agregar un valor aca (p.ej. PlaceNpc) + su Tool concreta es lo
 * unico necesario para sumar un tipo de edicion, sin tocar la logica existente.
 */
enum class ToolType { Pencil, Eraser, Fill, Teleport, Collision };

#endif // EDITOR_TOOLS_TOOLTYPE_H_
