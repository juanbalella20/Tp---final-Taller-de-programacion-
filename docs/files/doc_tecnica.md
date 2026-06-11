# Documentación Técnica — Argentum Online


## Tabla de contenidos

1. [Arquitectura general](#arquitectura-general)
2. [Componente: Servidor](#componente-servidor)
3. [Componente: Cliente](#componente-cliente)
4. [Componente: Editor de mapas](#componente-editor-de-mapas)
5. [Protocolo de comunicación](#protocolo-de-comunicación)
6. [Formato del archivo de mapa](#formato-del-archivo-de-mapa)
7. [Decisiones de diseño](#decisiones-de-diseño)

---

## Arquitectura general

El proyecto está organizado con una arquitectura cliente-servidor. El servidor es la fuente de verdad del estado del juego: los clientes no modifican el estado directamente, sino que envían intenciones o acciones al servidor.

```text
[Editor de mapas]
       |
       | exporta archivo de mapa
       v
[Archivo de mapa] -------- carga al iniciar --------> [Servidor]
                                                        ^
                                                        |
                                                [Protocolo de red]
                                                        |
                                                        v
                                        [Cliente 1] [Cliente 2] [Cliente N]

| Componente | Tecnología principal | Binario generado |
|------------|---------------------|-----------------|
| Servidor   | C++, [lib de red]   | `build/server`  |
| Cliente    | C++, [Qt/SFML/SDL]  | `build/client`  |
| Editor     | C++, Qt 6 Widgets   | `build/editor`  |
```

### Diagrama de paquetes

```
@startuml diagrama_paquetes_alto_nivel
title Diagrama de Paquetes - Argentum (vista de alto nivel)

skinparam packageStyle folder
skinparam shadowing false
skinparam package {
    BackgroundColor #F8F8F8
    BorderColor #555555
}

package "client" as client {
    [interface (GUI)]
    [comunication]
    [serializers]
    [audio]
    [clientApp]
}

package "server" as server {
    [Game]
    [communication]
    [serializer]
    [persistence]
    [gameloop]
}

package "common" as common {
    [socket]
    [commands]
    [constants]
    [info]
    [binaryMap]
    [utility]
}

client ..> common : <<import>>
server ..> common : <<import>>

note bottom of common
  common no depende de client ni de server.
  client y server NO se dependen entre si:
  se comunican por red usando los tipos
  compartidos definidos en common.
end note

@enduml
```

### Estructura del repositorio

```
[nombre-repo]/
├── server/             ← código del servidor
│   ├── src/
│   └── include/
├── client/             ← código del cliente
│   ├── src/
│   └── include/
├── editor/             ← código del editor de mapas
│   ├── src/
│   └── include/
├── common/             ← librería compartida (protocolo, tipos)
│   ├── src/
│   └── include/
├── maps/               ← mapas de ejemplo
├── assets/             ← recursos gráficos y de audio
└── CMakeLists.txt      ← build raíz
```

---

## Componente: Servidor

### Responsabilidades

El servidor es el componente autoritativo del juego. Es responsable de:

- Aceptar y gestionar conexiones de múltiples clientes simultáneos.
- Mantener el estado del mundo del juego (posiciones, entidades, etc.).
- Procesar los eventos enviados por los clientes y validarlos.
- Propagar los cambios de estado a todos los clientes conectados.
- Cargar y servir el mapa al inicio.

### Diagrama de clases — Servidor

```plantuml
@startuml
title Clases principales — Servidor

class GameServer {
  - port: int
  - world: GameWorld*
  - clients: vector<ClientHandler*>
  + start(): void
  + stop(): void
  - acceptConnections(): void
}

class ClientHandler {
  - socket: [TipoSocket]
  - playerId: uint32_t
  - world: GameWorld*
  + run(): void
  + sendMessage(msg: Message): void
  - receiveLoop(): void
  - handleMessage(msg: Message): void
}

class GameWorld {
  - players: map<uint32_t, Player>
  - map: GameMap*
  - mutex: [TipoMutex]
  + addPlayer(id): void
  + removePlayer(id): void
  + movePlayer(id, dir): Position
  + getState(): WorldState
  + broadcast(msg: Message): void
}

class GameMap {
  - tiles: vector<vector<int>>
  - width: int
  - height: int
  + loadFromFile(path): bool
  + isWalkable(x, y): bool
  + getTile(x, y): int
}

class Player {
  - id: uint32_t
  - x: int
  - y: int
  - [otros atributos]: [tipo]
}

GameServer *-- GameWorld
GameServer o-- ClientHandler
ClientHandler --> GameWorld
GameWorld *-- GameMap
GameWorld *-- Player
@enduml
```

### Diagrama de secuencia — Conexión de un cliente

```plantuml
@startuml
title Conexión de un nuevo cliente

actor Jugador
participant "GameClient" as C
participant "GameServer\n(Acceptor)" as S
participant "ClientHandler" as H
participant "GameWorld" as W

Jugador -> C: ejecuta el cliente
C -> S: TCP connect (ip:puerto)
S -> H: crea ClientHandler para el socket
H -> W: addPlayer(nuevo_id)
W --> H: ok
H -> C: MSG_WELCOME (player_id)
H -> W: getMapState()
W --> H: tiles iniciales
H -> C: MSG_MAP_DATA (tiles)
H -> W: getPlayers()
W --> H: lista de jugadores activos
H -> C: MSG_PLAYER_JOINED (por cada jugador)
C --> Jugador: muestra el juego
@enduml
```

### Diagrama de secuencia — Movimiento de un jugador

```plantuml
@startuml
title Movimiento de jugador — broadcast a todos los clientes

actor Jugador
participant "GameClient" as C
participant "ClientHandler\n(del jugador)" as H
participant "GameWorld" as W
participant "OtroClientHandler" as OH
participant "OtroCliente" as OC

Jugador -> C: presiona tecla de movimiento
C -> H: MSG_PLAYER_MOVE(direction)
H -> W: movePlayer(playerId, direction)
W -> W: valida colisión con el mapa
W --> H: nueva posición (x, y)
H -> C: MSG_ENTITY_UPDATE(id, x, y)
H -> OH: MSG_ENTITY_UPDATE(id, x, y)
OH -> OC: actualiza posición del jugador
@enduml
```

<!-- TODO: ajustar con el flujo real de movimiento. Si hay más eventos (ataque, chat), agregar diagramas similares -->

---

## Componente: Cliente

### Responsabilidades

- Conectarse al servidor y mantener la sesión activa.
- Capturar los inputs del usuario y enviarlos al servidor.
- Recibir el estado del juego del servidor y renderizarlo.
- Mostrar el mapa, los personajes y la interfaz de usuario.

### Diagrama de secuencia — Desde la ejecución del cliente hasta la partida

El siguiente diagrama muestra únicamente la colaboración entre el administrador
de pantallas, las pantallas iniciales y la sesión de autenticación.

![Flujo de las pantallas iniciales](../diagrams%20/%20client/3.svg)

### Diagrama de secuencia - Desconexión de un jugador

En este diagrama se muestra la desconexión de un jugador y la notificación al resto de players.

![Desconexión de un jugador](../diagrams%20/%20client/6.svg)


### Modelo de threading del cliente

[Descripción del modelo de threading del cliente.]

### Diagrama de clases — Cliente

![ClientGUI](../diagrams_png/clientGUI.png)
![ClientGUI e Info](../diagrams_png/clientGUI&info.png)
![ClientApp](../diagrams_png/clientApp.png)
![ClientProtocol](../diagrams_png/clientProtocol.png)


---
## Protocolo de comunicación

La comunicación entre servidor y clientes se realiza sobre **[TCP / UDP]** con un protocolo binario propio.

### Estructura de un paquete

Cada mensaje tiene el siguiente formato:

```
┌─────────────┬──────────────┬─────────────────────────┐
│  type (1B)  │  length (2B) │  payload (length bytes) │
└─────────────┴──────────────┴─────────────────────────┘
```

| Campo | Tipo | Tamaño | Descripción |
|-------|------|--------|-------------|
| `type` | `uint8` | 1 byte | Identificador del tipo de mensaje |
| `length` | `uint16` | 2 bytes | Longitud del payload en bytes |
| `payload` | `bytes` | variable | Datos del mensaje (ver tabla) |

### Tabla de mensajes

| ID | Nombre | Dirección | Payload |
|----|--------|-----------|---------|
| `0x01` | `MSG_PLAYER_MOVE` | Cliente → Servidor | `uint8 direction` (0=arriba, 1=abajo, 2=izq, 3=der) |
| `0x02` | `MSG_ENTITY_UPDATE` | Servidor → Cliente | `uint32 entity_id, int16 x, int16 y` |
| `0x03` | `MSG_CHAT_MESSAGE` | Cliente → Servidor | `uint16 len, char[] text` |
| `0x04` | `MSG_CHAT_BROADCAST` | Servidor → Cliente | `uint32 player_id, uint16 len, char[] text` |
| `0x05` | `MSG_PLAYER_JOINED` | Servidor → Cliente | `uint32 player_id, int16 x, int16 y` |
| `0x06` | `MSG_PLAYER_LEFT` | Servidor → Cliente | `uint32 player_id` |
| `0x10` | `MSG_WELCOME` | Servidor → Cliente | `uint32 assigned_id` |
| `0x11` | `MSG_MAP_DATA` | Servidor → Cliente | `uint16 width, uint16 height, uint16[] tile_ids` |
| `[0xNN]` | `[NOMBRE]` | [dirección] | [campos] |

### Manejo de desconexión


## Editor de mapas

### Objetivo y alcance

El editor es una aplicación de escritorio desarrollada con C++20 y Qt6 Widgets.
Permite:

- cargar spritesheets PNG como tilesets;
- seleccionar tiles mediante su identificador global (`gid`);
- editar dos capas: `ground` y `buildings`;
- usar lápiz, goma y relleno;
- marcar colisiones y teleports;
- abrir y guardar mapas en formato binario `.bin`.

El mapa tiene dimensiones fijas definidas en `game_constants.h`:

| Propiedad | Valor |
|---|---:|
| Ancho | 30 celdas |
| Alto | 16 celdas |
| Tamaño de tile | 64 px |
| Capas de tiles | 2 |

### Diagrama de clases central

Este diagrama se centra en `EditorDocument`, porque conecta los eventos de la
interfaz, las estrategias de edición y el modelo.

![Clases centrales del editor](../diagrams%20/editor/1.svg)

### Secuencia de edición de tiles

La siguiente secuencia representa lápiz y goma. El relleno termina después del
`press` porque `paints_on_drag()` devuelve `false`.

![Secuencia de edición de tiles](../diagrams%20/editor/4.svg)

#### Herramientas especiales

`Collision` y `Teleport` aparecen en la misma toolbar, pero no implementan
`Tool`:

- **Collision:** `EditorDocument` fija en el `press` el valor booleano que
  pintará durante todo el arrastre. Modifica `Map::collision_`.
- **Teleport:** funciona como toggle de una celda. Agrega o elimina un
  `TeleportDef{x, y, dest_zone}`.

En ambos casos se emite `cellChanged(-1,x,y)`. El valor `-1` indica que cambió
una superposición visual y no una capa de gids.

### Tilesets, selección y render

![Tilesets, selección y render](../diagrams%20/editor/2.svg)

#### `TileLibrary`

`TileLibrary::loadTileset(path,tileSize)`:

1. carga el PNG en `m_masterPixmap`;
2. calcula cuántas filas y columnas completas contiene;
3. recorta cada tile y lo guarda en `m_tiles`;
4. conserva dimensiones y ruta de origen.

`m_masterPixmap` sirve para mostrar la plancha completa en
`TilesetSelectorView`. `m_tiles` permite obtener directamente un tile mediante
su índice local.

La llamada a `m_tiles.reserve(columns * rows)` reserva capacidad para evitar
realocaciones durante los `push_back`; no crea elementos.

Si el ancho o alto del PNG no es múltiplo de `tileSize`, los píxeles sobrantes
se ignoran.

#### Identificadores locales y globales

Cada tileset ocupa un rango global: gid = firstgid + localId

El gid `0` está reservado para una celda vacía. `Map::next_firstgid()` asigna el
primer gid libre al registrar un tileset.

`TilesetSelectorView` transforma el click sobre el PNG en un `localId`, suma
`firstgid` y emite `tileSelected(gid, pixmap)`.

#### Caché de render

`MapEditorScene::rebuildTileCache()` recorre los tilesets registrados, carga
cada PNG mediante una `TileLibrary` temporal y construye:

```cpp
QHash<int, QPixmap> m_tileCache; // gid -> imagen lista para dibujar
```

Así, al cambiar una celda, `updateVisualItem()` puede resolver el pixmap por gid
sin volver a recortar el spritesheet.

### Persistencia: guardar y abrir

- abrir: `EditorDocument::open()` usa `BinaryMapLoader`;
- guardar: `EditorWindow::save_to()` usa `BinaryMapSaver`.

![Secuencia de persistencia](../diagrams%20/editor/5.svg)

Las rutas de PNG se guardan relativas a la carpeta de recursos compartida.
`paths::resource_relative()` prepara la ruta al guardar y
`paths::resolve_resource()` la convierte nuevamente en una ruta cargable

---
