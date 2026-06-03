#ifndef NIVEL_H_
#define NIVEL_H_

#include <cstdint>

class Nivel {
private:
    int numero;

    // OroMax = 100 * nivel^1.1
    uint32_t oro_max() const;

public:
    explicit Nivel(int numero);

    int get_numero() const;

    // Devuelve cuánto oro droppea al morir.
    // Si gold <= oro_max → 0 (nada cae)
    // Si gold >  oro_max → gold - oro_max (el exceso cae)
    uint32_t calcular_drop_gold(uint32_t gold) const;

    // XP que gana este nivel (atacante) por un golpe a un target de nivel_target.
    // Exp = dano * max(nivel_target - numero + 10, 0)
    int xp_por_ataque(int dano, int nivel_target) const;

    // XP adicional que gana este nivel (atacante) al matar al target.
    // Exp = rand(0, 0.1) * vida_max_target * max(nivel_target - numero + 10, 0)
    int xp_por_kill(int vida_max_target, int nivel_target) const;
};

#endif
