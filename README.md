# Tp---final-Taller-de-programacion-

Workflow para testear:

Para compilar Cmake:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build
```

Para correr:
```bash
Terminal 1: ./build/Debug/server 8080
Terminal 2: ./build/Debug/client localhost 8080 myplayer human warrior
```