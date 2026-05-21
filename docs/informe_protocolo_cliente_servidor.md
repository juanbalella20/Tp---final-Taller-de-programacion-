# Informe de protocolo cliente-servidor

Fecha: 21/05/2026
Proyecto: Argentum Online (TP Taller de Programación)

## 1) Objetivo
Este informe resume el estado actual del protocolo binario implementado entre cliente y servidor, incluyendo:
- formato de mensajes,
- serialización/deserialización en ambos extremos,
- cobertura real por tipo de mensaje,
- funcionalidades que hoy están completas de punta a punta.

## 2) Formato general del protocolo
Todos los mensajes usan un encabezado fijo de 3 bytes:
- `type` (1 byte): identifica el mensaje (`MessageType`),
- `payload_len` (2 bytes, big-endian): largo del payload.

Luego se envía el payload con largo variable.

Estado en código:
- Definición de constantes y enums: `common/protocol_constants.h`.
- Escritura de header con `htons`: `client/serialze.cpp` y `server/serverSerializer.cpp`.
- Lectura de header con `ntohs`: `client/clientProtocol.cpp` y `server/serverProtocol.cpp`.

## 3) Modelo de datos compartido
Se usa un modelo común para desacoplar red y lógica:
- `ClientCmd`: comando del cliente hacia el servidor (tipo, nombre, target, item, dirección, oro, etc.).
- `GameMsg`: evento del servidor hacia el cliente (tipo, dirección, mapa, nombre, coordenadas).

Esto permite que los threads de red solo traduzcan bytes <-> objetos y que el gameloop procese objetos de dominio.

## 4) Cliente -> Servidor

### 4.1 Serialización en cliente
`Serializer` implementa handlers por tipo y genera bytes según el formato de cada comando.

Tipos soportados en el serializador cliente:
- `MSG_REGISTER`
- `MSG_MOVE`
- `MSG_ATTACK`, `MSG_SELECT`
- `MSG_BUY`, `MSG_SELL`, `MSG_EQUIP`, `MSG_THROW`, `MSG_DEPOSIT`, `MSG_RETIRE`
- `MSG_MEDITATE`, `MSG_RESURRECT`, `MSG_CURE`, `MSG_LIST`, `MSG_REV_CLAN`, `MSG_LEFT_CLAN`
- `MSG_FOUND_CLAN`, `MSG_JOIN_CLAN`, `MSG_CLAN_ACEP`, `MSG_CLAN_RECH`, `MSG_CLAN_BAN`, `MSG_CLAN_KICK`
- `MSG_DEP_GOLD`, `MSG_RET_GOLD`

Formatos implementados:
- Register: `name_len + name + race + class`
- Move: `direction`
- Attack/Select: `entity_type + target_len + target_name`
- Comandos de clan por nombre: `target_len + target_name`
- Comandos de item: `item_len + item_id`
- Sin payload: header con `payload_len = 0`
- Oro: `uint32_t` en big-endian

### 4.2 Deserialización en servidor
`ServerDeserializer` reconstruye `ClientCmd` según `type`.

Handlers implementados en servidor:
- `MSG_REGISTER`, `MSG_MOVE`, `MSG_ATTACK`, `MSG_SELECT`
- `MSG_BUY`, `MSG_SELL`, `MSG_EQUIP`, `MSG_THROW`, `MSG_DEPOSIT`, `MSG_RETIRE`
- `MSG_DEP_GOLD`, `MSG_RET_GOLD`
- `MSG_MEDITATE`, `MSG_RESURRECT`, `MSG_CURE`, `MSG_LIST`, `MSG_REV_CLAN`, `MSG_LEFT_CLAN`

No implementados en `ServerDeserializer`:
- `MSG_LOGIN`, `MSG_TAKE`, `MSG_PRIVATE`
- `MSG_FOUND_CLAN`, `MSG_JOIN_CLAN`, `MSG_CLAN_ACEP`, `MSG_CLAN_RECH`, `MSG_CLAN_BAN`, `MSG_CLAN_KICK`

## 5) Servidor -> Cliente

### 5.1 Serialización en servidor
`ServerSerializer` hoy serializa:
- `MSG_MOVE` (payload: `direction`)
- `MSG_SEND_MAP` (payload plano de celdas codificadas)

Para `MSG_SEND_MAP`, cada celda se codifica con `ELEMENT_TYPE` y se envía en orden fila por fila.

### 5.2 Deserialización en cliente
`ClientDeserializer` hoy deserializa:
- `MSG_MOVE`
- `MSG_SEND_MAP`

Para `MSG_SEND_MAP` valida tamaño exacto `HEIGHT * WIDTH` y reconstruye `std::vector<std::vector<elements>>`.

## 6) Flujo de red y concurrencia
La arquitectura de red está separada por threads y colas:

### Cliente
- `NetworkSenderThread`: toma `ClientCmd` de cola y llama `ClientProtocol::send_command`.
- `NetworkReceiverThread`: recibe `GameMsg` desde socket y lo encola para UI.

### Servidor
- `ReceiverThread`: recibe `ClientCmd`, asigna `client_id` y encola para gameloop.
- `SenderThread`: toma `GameMsg` de cola por cliente y lo envía.
- `ClientRegistryMonitor`: mantiene registro `client_id -> cola de salida` y permite notificación individual o broadcast.

## 7) Cobertura funcional real (end-to-end)
Aunque hay muchos tipos serializados/deserializados, en lógica de juego (`GameLoop`) actualmente solo se procesan:
- `MSG_REGISTER`:
  - asigna nombre al `client_id`,
  - envía `MSG_SEND_MAP` al cliente que registró.
- `MSG_MOVE`:
  - intenta mover al jugador en `GameMap`,
  - si se movió, hace broadcast de `MSG_MOVE`.

En consecuencia, lo hoy operativo de punta a punta es:
1. Registro inicial (`REGISTER`) + envío de mapa (`SEND_MAP`).
2. Movimiento (`MOVE`) + difusión a clientes.

## 8) Tabla resumen por tipo de mensaje
Referencias:
- Definición de tipos: `common/protocol_constants.h`
- Serialización cliente: `client/serialze.cpp`
- Deserialización servidor: `server/serverDeserializer.cpp`
- Lógica de juego: `server/gameloop.cpp`

| Mensaje | Cliente serializa | Servidor deserializa | GameLoop procesa |
|---|---:|---:|---:|
| MSG_REGISTER | Si | Si | Si |
| MSG_LOGIN | No | No | No |
| MSG_MOVE | Si | Si | Si |
| MSG_ATTACK | Si | Si | No |
| MSG_TAKE | No | No | No |
| MSG_THROW | Si | Si | No |
| MSG_EQUIP | Si | Si | No |
| MSG_MEDITATE | Si | Si | No |
| MSG_RESURRECT | Si | Si | No |
| MSG_CURE | Si | Si | No |
| MSG_LIST | Si | Si | No |
| MSG_BUY | Si | Si | No |
| MSG_SELL | Si | Si | No |
| MSG_DEPOSIT | Si | Si | No |
| MSG_RETIRE | Si | Si | No |
| MSG_DEP_GOLD | Si | Si | No |
| MSG_RET_GOLD | Si | Si | No |
| MSG_PRIVATE | No | No | No |
| MSG_SELECT | Si | Si | No |
| MSG_FOUND_CLAN | Si | No | No |
| MSG_JOIN_CLAN | Si | No | No |
| MSG_REV_CLAN | Si | Si | No |
| MSG_CLAN_ACEP | Si | No | No |
| MSG_CLAN_RECH | Si | No | No |
| MSG_CLAN_BAN | Si | No | No |
| MSG_CLAN_KICK | Si | No | No |
| MSG_LEFT_CLAN | Si | Si | No |

## 9) Estado de la UI frente al protocolo
La GUI:
- envía `MSG_MOVE` con teclas de movimiento,
- al recibir eventos, solo aplica `MSG_MOVE` al player local.

Observación: aunque el cliente ya puede deserializar `MSG_SEND_MAP`, la GUI actual no consume ese mensaje en `update()` para dibujar desde red (el mapa se carga localmente desde TOML en `init_draw()`).

## 10) Conclusión
El protocolo binario base está correctamente estructurado y ya existe una implementación amplia de serialización/deserialización para muchos comandos.

A nivel funcional, el estado actualmente consolidado para demo es:
- registro de personaje,
- envío de mapa,
- movimiento con broadcast.

El resto de comandos ya tiene gran parte del “cableado” de protocolo (sobre todo Cliente -> Servidor), pero falta integrar procesamiento en `GameLoop` y/o completar handlers faltantes en servidor para cerrar cada caso end-to-end.
