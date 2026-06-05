#ifndef EDITOR_TOGGLE_TELEPORT_COMMAND_H
#define EDITOR_TOGGLE_TELEPORT_COMMAND_H

#include <string>

#include "Command.h"

class Map;

/*
 * Command concreto: marca/desmarca UNA celda como teleport (toggle), para
 * poder deshacer/rehacer la operacion.
 *
 * A diferencia de SetTilesCommand (que opera sobre la matriz de gids de una
 * capa), los teleports viven en un vector<TeleportDef> del Map: este Command
 * agrega/quita una entrada {x, y, dest_zone}.
 *
 * En construccion captura el estado previo de la celda (si habia teleport y con
 * que dest_zone) para que undo() lo restaure exactamente.
 */
class ToggleTeleportCommand : public Command {
public:
    // 'dest_zone' es la zona destino activa, usada si el toggle AGREGA.
    ToggleTeleportCommand(Map* map, int x, int y, std::string dest_zone);

    void execute() override;  // toggle: agrega (con dest_zone) o quita
    void undo() override;     // restaura el estado previo de la celda

private:
    Map* map_;
    int x_;
    int y_;
    std::string dest_zone_;  // zona a usar si execute() agrega

    // Estado previo de la celda (capturado en el constructor) para el undo.
    bool was_present_;
    std::string prev_zone_;
};

#endif  // EDITOR_TOGGLE_TELEPORT_COMMAND_H
