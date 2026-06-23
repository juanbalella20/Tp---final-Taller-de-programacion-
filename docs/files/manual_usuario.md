# Manual de Usuario — Argentum Online

## Tabla de contenidos

1. [Introducción](#introducción)
2. [Requisitos del sistema](#requisitos-del-sistema)
3. [Compilación del proyecto](#compilación-del-proyecto)
4. [Configuración inicial](#configuración-inicial)
5. [Modificar mapas y configuración sin recompilar](#modificar-mapas-y-configuración-sin-recompilar)
6. [Levantar el servidor](#levantar-el-servidor)
7. [Lanzar el cliente y jugar](#lanzar-el-cliente-y-jugar)
8. [Editor de mapas](#editor-de-mapas)

---

## Introducción

Este proyecto es una recreación del juego Argentum Online que incluye tres componentes:

- **Servidor**: gestiona el estado del mundo del juego y las conexiones de los clientes.
- **Cliente**: permite a los jugadores conectarse, ver el mapa y jugar.
- **Editor de mapas**: herramienta para crear y editar los mapas que usa el servidor.

Este manual explica paso a paso cómo compilar, configurar y usar cada componente.

---

## Requisitos del sistema

### Sistema operativo

- **Ubuntu/Debian**

---

## Compilación del proyecto

### Instalador automático

El proyecto incluye `install.sh`, que instala las
dependencias de sistema (apt), inicializa los submódulos de SDL, compila,
corre los tests unitarios e instala el juego en tu home:

```bash
# Clonar el repositorio (con submódulos)
git clone --recurse-submodules https://github.com/juanbalella20/Tp---final-Taller-de-programacion-.git

cd Tp---final-Taller-de-programacion-

# Ejecutar el instalador
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
`server`, `client` y `map_editor` desde cualquier directorio. El resto de este
manual asume que el juego se instaló de esta forma.

---

## Configuración inicial

### Archivos de configuración

#### Config.toml

```toml
# Ejemplo de configuración del servidor
[network]
host = "localhost"
port = "8080"
```

---

## Modificar mapas y configuración sin recompilar

Los mapas y la configuración **no forman parte del programa**: se leen cada vez que
arranca el juego. Por eso, para cambiar un mapa o ajustar la configuración **nunca hace
falta volver a compilar ni reinstalar**. Alcanza con editar el archivo correspondiente
y volver a iniciar el servidor y el cliente.

Tras instalar con `install.sh`, los archivos que podés editar están acá:

| Qué querés cambiar                  | Archivo a editar                                    |
|-------------------------------------|-----------------------------------------------------|
| Qué mapa usa cada zona, NPCs, etc.  | `~/.config/argentum/config.toml`                    |
| El contenido de un mapa             | `~/.local/share/argentum/data/maps/.../<mapa>.bin`  |

### Cambiar el mapa de una zona

1. Abrí `~/.config/argentum/config.toml` con cualquier editor de texto.
2. Buscá la zona que querés cambiar (sección `[zones.<nombre>]`).
3. Reemplazá el nombre del archivo en la línea `map = "..."` por el del mapa nuevo:

   ```toml
   [zones.desert]
   map = "data/maps/desert/desert-v4.bin"   # ← cambiá el nombre por el del mapa nuevo
   allowed_npcs = ["goblin", "spider"]
   # ...
   ```

4. Guardá el archivo y volvé a iniciar el servidor y el cliente. El nuevo mapa se carga
   automáticamente.

> 💡 El servidor y el cliente leen **el mismo** `config.toml`, así que con cambiarlo en
> un solo lugar alcanza: nunca quedan cargando mapas distintos.

---

## Levantar el servidor

> ⚠️ **El servidor debe iniciarse antes que cualquier cliente.**

### Iniciar el servidor

```bash
$ server
```

> El servidor toma host y puerto del `config.toml` (`[network]`); no recibe argumentos.

### Cargar un mapa específico

Para que el servidor arranque con otro mapa, se cambia el nombre del `.bin` en el
`config.toml`. Los pasos están en
[Cambiar el mapa de una zona](#cambiar-el-mapa-de-una-zona).

### Detener el servidor

Para cerrar el servidor limpiamente:

```bash
# Presionar 'q' en la terminal donde corre el servidor
```

---

## Lanzar el cliente y jugar

### Iniciar el cliente

Con el servidor ya corriendo, ejecutar:

```bash
$ client
```

> El cliente también lee host y puerto del `config.toml` (`[network]`); no recibe argumentos.

### Pantalla de inicio

![Pantalla de inicio](../../assets/screenshots/pantalla-inicio.png)

Seleccionar Jugar o Settings (resolución y pantalla)

![Sign-up](../../assets/screenshots/create-account.png)

Crear nuevo usuario

![Login](../../assets/screenshots/login.png)

Ingresar con cuenta existente

### Juego

![Juego en funcionamiento](../../assets/screenshots/in-game.png)

### Controles básicos

| Tecla / Acción | Resultado |
|----------------|-----------|
| `W` / `↑` | Mover hacia arriba |
| `S` / `↓` | Mover hacia abajo |
| `A` / `←` | Mover hacia la izquierda |
| `D` / `→` | Mover hacia la derecha |

## Editor de mapas

### Iniciar el editor

```bash
$ map_editor
```

### Interfaz del editor

![Editor de mapas](../../assets/screenshots/editor.png)

La interfaz está dividida en las siguientes secciones:

| Sección | Descripción |
|---------|-------------|
| [Panel izquierdo] | Visualización de los tilesets |
| [Canvas central] | Canvas para crear el mapa utilizando los tilesets |
| [Barra de herramientas 1] | Lápiz, goma, relleno |
| [Barra de herramientas 2] | Teleport, colisión: permiten pintar un tile indicando si es colisionable o teleport |
| [Barra de herramientas 3] | Capa en la que se pinta. Tres opciones: **Suelo** (tile base), **Construcciones** (se pinta sobre el suelo, detrás del jugador) y **Decoración** (se pinta delante del jugador) |
| [Panel de tilesets] | Combo para intercambiar entre los tilesets ya cargados |

> Los tilesets **se cargan solos** al abrir el editor: toma todos los PNG de
> `~/.local/share/argentum/assets/tilesets/`. Para
> sumar un tileset, copiá el PNG a esa carpeta y reiniciá el editor.

### Crear un mapa nuevo

1. Abrí el editor: ya vas a tener todos los tilesets disponibles en el combo del panel.
2. Elegí un tileset en el combo del panel de tilesets.
3. Seleccioná un tile, una herramienta y empezá a pintar.
4. Guardá con **Archivo > Guardar** dentro de `~/.local/share/argentum/data/maps/<zona>/<nombre-mapa>.bin`.

### Editar mapa existente

1. En el editor, andá a **Archivo > Abrir**.
2. Buscá el mapa en `~/.local/share/argentum/data/maps/` (por ejemplo, `city/city-v2.bin`).
3. Editá el mapa con las herramientas del editor.
4. Guardá con **Archivo > Guardar**. Según cómo guardes:
   - **Con el mismo nombre** (pisás el archivo anterior): listo, solo volvé a iniciar
     el servidor y el cliente para ver los cambios.
   - **Con otro nombre** (por ejemplo, `city-v2.bin`): además tenés que indicarle al
     juego que use el archivo nuevo, editando el `config.toml`
     (ver [Cambiar el mapa de una zona](#cambiar-el-mapa-de-una-zona)).

### Drag & Drop

1. Selecciona un tile
2. Presiona shift y selecciona los tiles deseados
3. Arrastra la selección al canva
![drag_drop](../../assets/gifs/drag-drop-tutorial.gif)

### Cambiar de tileset

![cambiar-tileset](../../assets/gifs/tilesets-tutorial.gif)