#include "nivel.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

Nivel::Nivel(int numero) : numero(numero) {}

int Nivel::get_numero() const {
    return numero;
}

uint32_t Nivel::oro_max() const {
    return static_cast<uint32_t>(100 * std::pow(numero, 1.1));
}

uint32_t Nivel::calcular_drop_gold(uint32_t gold) const {
    uint32_t max = oro_max();
    if (gold <= max) return 0;
    return gold - max;
}

int Nivel::xp_por_ataque(int dano, int nivel_target) const {
    int factor = std::max(nivel_target - numero + 10, 0);
    return dano * factor;
}

int Nivel::xp_por_kill(int vida_max_target, int nivel_target) const {
    int factor = std::max(nivel_target - numero + 10, 0);
    double rnd = (std::rand() / static_cast<double>(RAND_MAX)) * 0.1;  // rand(0, 0.1)
    return static_cast<int>(rnd * vida_max_target * factor);
}
