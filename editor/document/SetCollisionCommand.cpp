#include "SetCollisionCommand.h"

#include "../model/Map.h"

SetCollisionCommand::SetCollisionCommand(Map *map,
                                         std::vector<BlockedChange> changes)
    : map_(map), changes_(std::move(changes)) {}

void SetCollisionCommand::execute() {
  for (const BlockedChange &c : changes_) {
    map_->set_blocked(c.x, c.y, c.new_blocked);
  }
}

void SetCollisionCommand::undo() {
  for (const BlockedChange &c : changes_) {
    map_->set_blocked(c.x, c.y, c.old_blocked);
  }
}
