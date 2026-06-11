# Escenario PvP: J1 mata a J2 de un golpe

Documento de apoyo para los diagramas de secuencia `pvp_kill_*.puml`. Define la
**situación concreta** (con números reales del juego, tomados de `config.toml`)
que ilustran los diagramas. Solo el **bloque 1** (1a/1b/1c) describe esta
situación de partida; los demás (2a/2b/2c, 3a/3b) la dan por establecida.

> **Alcance:** estos diagramas modelan **solo la parte del juego (server-side)**.
> Quedan deliberadamente fuera el cliente (click, `ClientGUI`, HUD/sprites) y la
> capa de red/serialización (serializers, `ClientProtocol`, sockets). La
> secuencia del ataque arranca en el `GameLoop` cuando saca el comando de la cola
> y termina cuando el server entrega los mensajes resultantes.

Todas las fórmulas y valores salen de `config.toml` y del código del servidor
(`Player`, `Level`, `DefenseSet`, `DefenseItem`, `AliveState`).

---

## Los dos jugadores

### J1 — atacante  (`j1:Player`)

| Atributo            | Valor                | De dónde sale |
|---------------------|----------------------|---------------|
| Raza / Clase        | Gnome / Warrior      | `[races.gnome]`, `[classes.warrior]` |
| Nivel               | **13**               | dejó de ser newbie (newbie ≤ 12) |
| Fuerza              | **17**               | `race_strength` 8 + `class_strength` 9 |
| Arma equipada       | **Arco compuesto**   | `[items.weapons.arco_compuesto]`, daño 4–16 |
| Oro                 | algo (irrelevante)   | no muere, no dropea |

> **Daño Z (según enunciado):** `Z = Fuerza * rand(daño_arma_min, daño_arma_max)`
> = `17 * rand(4, 16)` ⇒ rango **68 … 272**.
> Para el golpe que ocurre en los diagramas fijamos el roll en **rand = 10**, así
> **Z = 17 * 10 = 170**.


### J2 — víctima  (`j2:Player`)

| Atributo            | Valor                | De dónde sale |
|---------------------|----------------------|---------------|
| Raza / Clase        | Human / Wizard       | `[races.human]`, `[classes.wizard]` |
| Nivel               | **13**               | |X−Y| = 0 ≤ 10 ✓ ; no newbie ✓ |
| **Vida máxima**     | **65**               | ver fórmula abajo |
| Vida al momento del golpe | **65** (full)  | |
| **Casco**           | **Casco de hierro**  | `[items.helmets.casco_hierro]`, defensa 4–8 |
| **Armadura**        | **Armadura de cuero**| `[items.armors.armadura_cuero]`, defensa 2–6 |
| Escudo              | (sin escudo)         | |
| Oro                 | **300**              | excede el oro seguro de su nivel ⇒ dropeará el excedente |

> **Vida máxima de J2:**
> `max_life = constitución * FClaseVida * FRazaVida * nivel`
> = `endurance(0.5) * class_life_factor(2) * race_life_factor(5) * 13` = **65**.

---

## Fair play: ¿por qué pueden atacarse? (X = 13, Y = 13)

Validado en `GameMap::attack` antes de aplicar el golpe:

| Regla (`config.toml`)                     | Chequeo                         | Resultado |
|-------------------------------------------|---------------------------------|-----------|
| Zona segura (`safe_zone`)                 | atacan fuera de `city`          | OK (no es segura) |
| Newbie (`newbie_max_level = 12`)          | J1 = 13 y J2 = 13, ninguno ≤ 12 | OK |
| Diferencia de nivel (`level_diff_max = 10`)| \|13 − 13\| = 0 ≤ 10            | OK |
| Mismo clan                                | J1 y J2 en clanes distintos     | OK |

---

## El golpe: por qué muere de uno solo (aun con casco + armadura)

`AliveState::receive_damage` sobre J2:

1. **Esquive** (solo si no es crítico): `rand(0,1)^Agilidad < dodge_threshold (0.001)`.
   En el escenario **no esquiva**.
2. **Defensa** = suma de los `DefenseItem` equipados, cada uno
   `defense_min + rand(0, max−min)`:
   - Casco de hierro: `4 + rand(0,4)` ⇒ fijamos **6**
   - Armadura de cuero: `2 + rand(0,4)` ⇒ fijamos **4**
   - **Defensa total = 10**
3. **Daño neto** = `max(0, Z − defensa)` = `max(0, 170 − 10)` = **160**.
4. Como `lives (65) ≤ daño (160)` ⇒ `lives = 0`, **`murio = true`**.

**160 de daño contra 65 de vida: muere de un solo golpe**, aun teniendo casco y
armadura equipados (la defensa de 10 no alcanza ni de lejos).

---

## Consecuencias de la muerte de J2

- **XP para J1** (`Player::ganar_xp` → `Level`):
  - `xp_per_attack = daño * max(Ytarget − Xnivel + xp_level_offset, 0)`
    = `160 * max(13 − 13 + 10, 0)` = `160 * 10` = **1600 XP** por el golpe.
  - `xp_per_kill = rand(0, xp_kill_factor=0.1) * VidaMaxJ2(65) * factor(10)`
    ⇒ bonus aleatorio adicional por la kill.
  - `check_level_up`: si la XP acumulada de J1 supera `1000 * nivel^1.8`, **sube de nivel**.
- **Oro que dropea J2** (`Level::calculate_gold_drop`):
  - Oro seguro de nivel 13 = `gold_base(100) * 13^1.1` ≈ **1.7k**… (si su oro no lo
    supera, no dropea oro). En este escenario asumimos que J2 cargaba oro por
    encima de su tope, así que **suelta el excedente** en su celda.
- **Items que dropea J2** (`Player::drop_inventory` → `DefenseSet::clear` +
  `Inventory::drop_all`): caen al piso **todos** sus items, incluidos el
  **casco de hierro** y la **armadura de cuero**.
- **Estado de J2**: pasa a **`GhostState`** (fantasma) en el servidor.

---

## Mapa de diagramas

| Diagrama | Archivo | Qué muestra |
|----------|---------|-------------|
| 1a | `pvp_kill_1a_init_server.puml` | **Esta situación**: arranque del server (`GameLoop`, `load_world`, `init_world`) |
| 1b | `pvp_kill_1b_spawn_j1.puml` | Spawn + equipamiento de J1 (atacante): `spawn_player` + arco compuesto |
| 1c | `pvp_kill_1c_spawn_j2.puml` | Spawn + equipamiento de J2 (víctima): `spawn_player` + casco y armadura |
| 2a | `pvp_kill_2a_gamemap_validacion.puml` | `GameMap::attack`: ubicar target + fair play |
| 2b | `pvp_kill_2b_golpe_y_dano.puml` | Delegación del golpe: `attack` → `Inventory` → `Item` → `receive_damage` (defensa, muerte) |
| 2c | `pvp_kill_2c_drop.puml` | Tras la muerte: drop de oro e inventario, armado del `AttackResult` |
| 3a | `pvp_kill_3a_broadcast_muerte.puml` | `handle_attack`: broadcast de `MSG_DEATH` a todos |
| 3b | `pvp_kill_3b_mensajes_victima.puml` | Mensajes dirigidos a J2: `MSG_HP`, `MSG_CHAT`, `MSG_GOLD`, `MSG_INVENTORY`, equipo |

> El ataque arranca en el `GameLoop` (al sacar el comando `MSG_ATTACK` de la cola
> y despachar `handle_attack`), no se modela cómo J1 generó ese comando desde el
> cliente. La aplicación de los mensajes en el cliente de la víctima tampoco se
> modela: la secuencia termina cuando el server los entrega.
