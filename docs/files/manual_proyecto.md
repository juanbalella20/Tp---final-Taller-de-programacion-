# Manual de Proyecto — [Nombre del Proyecto]

## Tabla de contenidos

1. [Integrantes del equipo](#integrantes-del-equipo)
2. [División de trabajo](#división-de-trabajo)
3. [Cronograma](#cronograma)
4. [Herramientas utilizadas](#herramientas-utilizadas)
5. [Recursos de aprendizaje](#recursos-de-aprendizaje)
6. [Retrospectiva](#retrospectiva)
7. [Sugerencias para la materia](#sugerencias-para-la-materia)

---

## Integrantes del equipo

| Nombre | Padrón | Contacto |
|--------|--------|----------|
| Juan Balella | 110271 | juanbalella20 |
| Agustín Trombetta | 111754 | agus778 |
| Mercedes González Lago | 110796 | mercedesgonzalezlago |
| Ignacio Mahmoud Abalos | 112322 | Ignaci0MA |


---

## División de trabajo

### ¿Quién hizo qué?

#### Agustín Trombetta

- Editor gráfico + Creación de mapas
- Lógica teleports
- Lógica zonas
- Lógica de spawn de enemigos por zonas
- Lógica pantallas iniciales
   - Login/signup
   - Panel creacion de personaje
   - settings de la pantalla
- Seguimiento de la cámara al player
- Lógica de los tiles y matriz
   - Dos tipos de tiles en mapa: Colisiones y Buildings, NPCs
- Participación en lógica del juego

#### Juan Balella
- logica de ataque
- logica de muerte
- logica meditacion
  - Estados del jugador
  - logica de dropeo => algoritmo de Cheryshev
- logica de defensa
- logica de autospawn de npc
- clases multimedia como musica y sonido
- cambio a multijugador
- estructura del protocolo del cliente al servidor(la vuelta fue realizada entre  por todos)

#### Mercedes González Lago
- Implementación MiniChat
- Visualización de sprites para jugadores (distintas razas)
- Visualización de sprites para npc hostiles (distintos tipos)
- Movimiento (cambio de sprites) de jugadores
- Colocación de item sobre el jugador
- Implementación tomar objetos
- Implementación clanes
- Movimiento (cambio de sprites) de npc hostiles
- Ataque de npc hostiles a jugadores
- Visualización de stats e info (oro, nombre y nivel) del jugador

#### Ignacio Mahmoud Abalos
- Modelacion items
- Logica inicial de gameloop
- Implementacion npc priest
- Implementacion npc seller
- Implementacion npc banker
- Visualizacion de tienda junto a Juan Balella
- Configuracion toml

---

## Cronograma

### Plan original vs realidad

| Fechas | Plan original | Lo que hicimos |
|--------|---------------|---------------------------|
| 10/05 al 17/05 | Realizar la planificación inicial y definir cómo se conectarían los distintos componentes del trabajo. Comenzar la lógica del juego (jugador, ítems y NPC), diseñar la estructura del protocolo y explorar SDL3. | Completamos la planificación y la visión general de la arquitectura. Implementamos las bases de la lógica del jugador, los ítems y los NPC, definimos la estructura del protocolo y realizamos una exploración inicial de SDL3. |
| 18/05 al 24/05 | Integrar el mapa a la matriz; implementar colisiones, spawn e interacción con NPC, combate y detección de proximidad de enemigos. Incorporar el minichat, cheats, eventos y comandos del servidor, seguimiento de cámara y comenzar el editor. | **Completado:** integración y refactor del mapa, colisiones, spawn y autospawn de NPC, interacción mediante clic, minichat, comandos cliente-servidor, eventos de respawn, seguimiento de cámara y combate contra NPC. También implementamos los cheats, excepto los de vida y maná infinitos, y planificamos el editor.<br>**Pendiente:** detección de proximidad de NPC enemigos e inicio de la implementación del editor. |
| 25/05 al 31/05 | Implementar el comportamiento de NPC hostiles y amistosos, incluyendo movimiento, proximidad, ataques, sprites y límites por zona. Incorporar la muerte del jugador como fantasma, el cambio de zona, el HUD y la visualización y sincronización de otros jugadores. | **Completado:** acciones de los NPC amistosos, ataques de NPC, seguimiento de cámara, cambio entre zonas, spawn de otros jugadores, HUD y visualización de los movimientos y acciones de otros jugadores.Se realizo una estructura basica  del editor de mapas<br>**Pendiente:** movimiento de NPC hostiles, detección de proximidad de enemigos, límite máximo de NPC por zona o mapa, aparición del jugador como fantasma al morir y sprites correspondientes para los NPC amistosos. |
| 01/06 al 07/06 | Implementar configuración, persistencia, clanes, pantalla inicial, niveles y experiencia, autoataque de NPC y finalizar el editor. Agregar las interfaces de tienda y banquero, además del sonido. | **Completado:** configuración, persistencia, clanes, pantalla inicial, lógica de niveles y experiencia, autoataque de NPC y finalización del editor.<br>**Pendiente:** visualización de la tienda y el banquero, música y sonidos.
 |
| 08/06 al 11/06 | Completar la música y los sonidos, y terminar la presentación y el movimiento de todos los NPC. | Incorporamos la música y los sonidos. También completamos los sprites correspondientes y el movimiento de todos los NPC. |

### Lo que no se llegó a realizar 


- Movimiento con predicción
- Que no tire la animacion de ataque, si no tiene mana suficiente
- Movimiento de sprites de las armas
- Algunos movimientos de sprites de NPCs

---

## Herramientas utilizadas

### IDEs y editores

| Herramienta | Uso |
|-------------|-----|
| VSCode      | Desarrollo general |
| QT creator  | Exploración de librería QT en desarrollo del editor |



## Recursos

### Documentación oficial

- [Qt 6 Documentation](https://doc.qt.io/qt-6/)
- [cppreference.com](https://en.cppreference.com)
- [SDL3 Wiki](https://wiki.libsdl.org/SDL3/FrontPage)
- [Cmake](https://cmake.org/cmake/help/latest/guide/tutorial/index.html)

### Libros

- [Game programming Patterns](https://gameprogrammingpatterns.com/contents.html) — [Robert Nystrom]

### Tutoriales y otros recursos

- [How To Make a Tile Editor in C++](https://youtu.be/hBwhiaxLAJM)
- [Video-Sobre-Tiled](https://youtu.be/f212jaHpXNo?si=h7K9fef16f9dF1eH)
- [Tiled-Repo](https://github.com/mapeditor/tiled)
- [QT-Designer-Tutorials](https://www.youtube.com/playlist?list=PL6CJYn40gN6iFcTyItvnE5nOmJR8qk_7o)
- [QT-Videos](https://training.kdab.com/)
- [QT-Widgets](https://youtu.be/RyJqcw0RXxk?si=sVZ-wCfwWrnSPJpz)
- [Creating debian packages — CMake](https://karthikkalyanaraman.medium.com/creating-debian-packages-cmake-e519a0186e87)
- [Packaging a C/C++ Application into a .deb package for Debian/Ubuntu: An Easy Guide for Beginners](https://medium.com/codecurrent/packaging-a-c-c-application-into-a-deb-package-for-debian-ubuntu-an-easy-guide-for-beginners-651665000660)
[Global Tile IDs](https://doc.mapeditor.org/en/stable/reference/global-tile-ids/)
[Flood Fill](https://www.algorithm-archive.org/contents/flood_fill/flood_fill.html)
[Flood Fill 2](https://usaco.guide/silver/flood-fill?lang=cpp)

