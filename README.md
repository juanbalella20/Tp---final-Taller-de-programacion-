# Tp---final-Taller-de-programacion-

Integrantes:

Juan Ballela
Agustín Trombetta
Mercedes González Lago
Ignacio Mahmoud Abalos

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
# Documentacion 

[Documento tecnico](docs/files/doc_tecnica.md)
[Manual proyecto](docs/files/manual_proyecto.md)
[Manual de usuario](docs/files/manual_usuario.md)

# Video promocional
https://drive.google.com/file/d/1kqbRD-1v4atTtIH8835QUMWhTmjKflAc/view?usp=sharing
