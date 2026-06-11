#include "escudo.h"

Escudo::Escudo(const std::string& id, const std::string& name, int price,
               int defense_min, int defense_max):
        DefenseItem(id, name, price, defense_min, defense_max) {}
