# Manual de Usuario — [Nombre del Proyecto]

> **Cómo usar este template:** Reemplazá todo lo que está entre corchetes `[así]` con la información real.
> Las líneas marcadas con `<!-- TODO -->` son las que tenés que completar o borrar si no aplica.

---

## Tabla de contenidos

1. [Introducción](#introducción)
2. [Requisitos del sistema](#requisitos-del-sistema)
3. [Instalación de dependencias](#instalación-de-dependencias)
4. [Compilación del proyecto](#compilación-del-proyecto)
5. [Configuración inicial](#configuración-inicial)
6. [Levantar el servidor](#levantar-el-servidor)
7. [Lanzar el cliente y jugar](#lanzar-el-cliente-y-jugar)
8. [Editor de mapas](#editor-de-mapas)
9. [Solución de problemas frecuentes](#solución-de-problemas-frecuentes)

---

## Introducción

[Nombre del Proyecto] es una recreación del juego Argentum Online que incluye tres componentes:

- **Servidor**: gestiona el estado del mundo del juego y las conexiones de los clientes.
- **Cliente**: permite a los jugadores conectarse, ver el mapa y jugar.
- **Editor de mapas**: herramienta para crear y editar los mapas que usa el servidor.

Este manual explica paso a paso cómo compilar, configurar y usar cada componente.

---

## Requisitos del sistema

### Sistema operativo

- **Ubuntu/Debian** (recomendado y probado)

---

## Instalación de dependencias

Antes de compilar, instalá todas las dependencias ejecutando el siguiente comando:

```bash
sudo apt update
sudo apt install -y \
    build-essential cmake pkg-config \
    qt6-base-dev \
    libasound2-dev libpulse-dev \
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev \
    libxfixes-dev libxi-dev libxss-dev libxtst-dev \
    libxkbcommon-dev libdrm-dev libgbm-dev \
    libgl1-mesa-dev libegl1-mesa-dev \
    libwayland-dev wayland-protocols libdecor-0-dev \
    libpipewire-0.3-dev libudev-dev
```

Las bibliotecas SDL3, SDL3_image, SDL3_ttf y SDL3_mixer están incluidas en el
repositorio como submódulos de Git y se compilan junto con el proyecto. No es
necesario instalar paquetes `libsdl` del sistema.

### Tabla de dependencias

| Dependencia | Versión mínima | Comando de instalación |
|-------------|----------------|------------------------|
| GNU G++ | 11, con soporte para C++17 | `sudo apt install build-essential` |
| CMake | 3.16 | `sudo apt install cmake` |
| pkg-config | Versión provista por el sistema | `sudo apt install pkg-config` |
| Qt6 Widgets | 6.x | `sudo apt install qt6-base-dev` |
| Soporte de audio | Versión provista por el sistema | `sudo apt install libasound2-dev libpulse-dev libpipewire-0.3-dev` |
| Soporte gráfico X11 | Versión provista por el sistema | `sudo apt install libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev` |
| Soporte gráfico Wayland | Versión provista por el sistema | `sudo apt install libwayland-dev wayland-protocols libdecor-0-dev libxkbcommon-dev` |
| Mesa, DRM y dispositivos | Versión provista por el sistema | `sudo apt install libdrm-dev libgbm-dev libgl1-mesa-dev libegl1-mesa-dev libudev-dev` |

---

## Compilación del proyecto

### Opción A — Instalador automático (recomendado)

El proyecto incluye `install.sh`, que hace todo de punta a punta: instala las
dependencias de sistema (apt), inicializa los submódulos de SDL, compila,
corre los tests unitarios e instala el juego en tu home:

```bash
# Clonar el repositorio (con submódulos)
git clone --recurse-submodules https://github.com/[usuario]/[repositorio].git
cd [repositorio]

# Ejecutar el instalador (pide sudo para apt; no hace falta chmod)
bash install.sh
```

Al terminar, todo queda instalado así:

| Qué | Dónde |
|-----|-------|
| Binarios (`client`, `server`, `map_editor`) | `~/.local/bin/` |
| Bibliotecas SDL3 | `~/.local/lib/` |
| Assets (imágenes, fuentes, mapas, audio) | `~/.local/share/argentum/` |
| Configuración (`config.toml`) | `~/.config/argentum/` |

Si `~/.local/bin` está en tu `PATH` (en Ubuntu suele estarlo), podés correr
`server 8080`, `client` y `map_editor` desde cualquier directorio.

### Binarios generados (compilación manual)

Tras compilar, los ejecutables quedan en:

| Binario | Ruta | Descripción |
|---------|------|-------------|
| Servidor | `build/Debug/server` | Servidor del juego |
| Cliente  | `build/Debug/client` | Cliente del juego |
| Editor   | `build/Debug/map_editor` | Editor de mapas |

> Con la compilación manual, ejecutá los binarios **desde la raíz del repo**
> (los assets y `config.toml` se buscan primero en `~/.local/share/argentum/` y
> `~/.config/argentum/`, y si no existen, en el directorio actual).

---

## Configuración inicial

### Recursos gráficos y de audio

Los archivos de recursos (tiles, sprites, sonidos) deben estar en la siguiente ubicación antes de ejecutar cualquier componente:

```
[repositorio]/
├── assets/
│   ├── tiles/          ← tilesets del mapa
│   ├── sprites/        ← sprites de personajes y objetos
│   └── sounds/         ← efectos de sonido
<!-- TODO: ajustar la estructura real de carpetas -->
```

### Archivos de configuración

#### Servidor — `[ruta/config_servidor]`

```ini
# Ejemplo de configuración del servidor
port = 7171
max_players = [N]
map_file = maps/[nombre_mapa_default].map
<!-- TODO: completar con todos los parámetros reales -->
```

| Parámetro | Descripción | Valor por defecto |
|-----------|-------------|-------------------|
| `port` | Puerto TCP que escucha el servidor | `7171` |
| `max_players` | Máximo de jugadores simultáneos | `[N]` |
| `map_file` | Ruta al mapa que carga al iniciar | `[valor]` |
<!-- TODO: completar con todos los parámetros reales -->

#### Cliente — `[ruta/config_cliente]`

<!-- TODO: completar con el archivo de configuración real del cliente, si existe -->

```ini
server_ip = 127.0.0.1
server_port = 7171
<!-- TODO: completar con todos los parámetros reales -->
```

---

## Levantar el servidor

> ⚠️ **El servidor debe iniciarse antes que cualquier cliente.**

### Iniciar el servidor

```bash
./build/server [opciones]
```

<!-- TODO: documentar las opciones disponibles, por ejemplo:
./build/server --port 7171 --map maps/default.map
-->

Opciones disponibles:

| Opción | Descripción | Ejemplo |
|--------|-------------|---------|
| `[opción 1]` | [descripción] | `[ejemplo]` |
| `[opción 2]` | [descripción] | `[ejemplo]` |

<!-- TODO: si no hay opciones por línea de comandos, borrar la tabla -->

### Output esperado al iniciar

Cuando el servidor arranca correctamente, deberías ver algo como:

```
[TODO: pegar aquí el output real de la consola cuando el servidor inicia bien]
```

### Cargar un mapa específico

<!-- TODO: explicar cómo se especifica el mapa a cargar. ¿Argumento? ¿Config? -->

```bash
./build/server [--map ruta/al/mapa.map]
```

### Detener el servidor

Para cerrar el servidor limpiamente:

```bash
# Presionar Ctrl+C en la terminal donde corre el servidor
```

<!-- TODO: si hay un comando especial o señal, documentarlo -->

---

## Lanzar el cliente y jugar

### Iniciar el cliente

Con el servidor ya corriendo, ejecutar:

```bash
./build/client [opciones]
```

<!-- TODO: documentar opciones, por ejemplo --host y --port -->

### Pantalla de inicio

<!-- TODO: insertar screenshot de la primera pantalla que ve el usuario -->

![Pantalla de inicio](assets/screenshots/pantalla_inicio.png)

<!-- TODO: describir qué hace el usuario en esta pantalla (ingresar nombre, IP del servidor, etc.) -->

### Pantalla principal del juego

<!-- TODO: insertar screenshot del juego en funcionamiento con el mapa visible -->

![Juego en funcionamiento](assets/screenshots/juego_principal.png)

### Controles

| Tecla / Acción | Resultado |
|----------------|-----------|
| `W` / `↑` | Mover hacia arriba |
| `S` / `↓` | Mover hacia abajo |
| `A` / `←` | Mover hacia la izquierda |
| `D` / `→` | Mover hacia la derecha |
| `[tecla]` | [acción] |
| `[tecla]` | [acción] |
| `Esc` | [acción — salir, menú, etc.] |

<!-- TODO: completar con todos los controles reales del juego -->

## Editor de mapas

### Iniciar el editor

```bash
./build/editor [opciones]
```

### Interfaz del editor

![Editor de mapas](assets/screenshots/editor1.png)

La interfaz está dividida en las siguientes secciones:

| Sección | Descripción |
|---------|-------------|
| [Panel izquierdo / Tileset] | Visualización de los tilesets |
| [Canvas central] | Canvas para crear el mapa utilizando los tiles |
| [Barra de herramientas 1] | Lápiz, goma, relleno |
| [Barra de herramientas 2] | Teleport, colisión: permiten pintar un tile indicando si es colisionable o teleport |
| [Barra de herramientas 3] | Suelo, Construcciones: intercalar entre tile para el suelo o tile para una construcción (se pinta arriba del tile de suelo) |
| [Panel de tilesets] | Se puede intercambiar entre distintos tilesets, cargar tilesets |

### Crear un mapa nuevo

1. Paso 1: Cargar PNG
2. Paso 2: Buscar tileset dentro de la carpeta del juego: `/../data/maps`
3. Paso 3: Seleccionar un tile, una herramienta y empezar a pintar
4. Paso 4: Archivo > Guardar: dentro de la carpeta del juego: `../data/maps/<mapa>`

### Cargar un nuevo mapa

Una vez guardado el mapa:
1. Ir a `/config.toml`
2. Buscar la sección zonas
3. Modifiar el nombre de map por el nuevo nombre del archivo
4. Guardar cambio en el config.toml

---

## Solución de problemas frecuentes

### Error al compilar: "Qt6 not found"

```
CMake Error: Could not find Qt6
```

**Solución:**
```bash
sudo apt install qt6-base-dev qt6-tools-dev
```

### El cliente no puede conectarse al servidor

**Verificar:**
1. Que el servidor esté corriendo antes de lanzar el cliente.
2. Que la IP y el puerto en la configuración del cliente coincidan con los del servidor.
3. Que no haya un firewall bloqueando el puerto `[puerto]`.

### [Error frecuente 3]

<!-- TODO: completar con errores reales que encontraron durante el desarrollo -->

```
[mensaje de error]
```

**Solución:** [descripción]

### [Error frecuente 4]

```
[mensaje de error]
```

**Solución:** [descripción]
