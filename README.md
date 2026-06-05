# Tp---final-Taller-de-programacion-

Workflow para testear juego:

Para compilar Cmake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON 

cmake --build build
```

Nota:

- El primer `cmake -S . -B build ...` inicializa automáticamente los submódulos vendoreados (`SDL`, `SDL_image`, `SDL_ttf`) y configura `SDL_ttf` con dependencias vendoreadas, sin requerir instalar `libsdl*` o `freetype` del sistema.
- Requiere conexión a internet y `git` disponible la primera vez.

Para correr:

```bash
Terminal 1: ./build/Debug/server 8080
Terminal 2: ./build/Debug/client localhost 8080 myplayer human warrior
```

Workflow para testear editor-grafico:

```bash

# 1. Configurar cmake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# 2. Buildear el editor
cmake --build build --target map_editor

# 3. Ejecutar
./build/Debug/map_editor
```
