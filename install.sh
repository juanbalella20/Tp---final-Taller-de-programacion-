#!/usr/bin/env bash
# Instalador del TP (argentum).
#
# Hace, en orden:
#   1. Instala las dependencias de sistema (apt).
#   2. Inicializa los submodulos de git (SDL3 vendoreado).
#   3. Compila el proyecto (CMake, Release).
#   4. Corre los tests unitarios (ctest).
#   5. Instala:  binarios -> ~/.local/bin
#                assets   -> ~/.local/share/argentum/
#                config   -> ~/.config/argentum/
#
# Uso:  bash install.sh
# (no hace falta chmod: se invoca via bash)

set -euo pipefail

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${REPO_DIR}/build"

step() { echo; echo "==> $1"; }

# ── 1. Dependencias de sistema ───────────────────────────────────────────────
step "Instalando dependencias de sistema (requiere sudo)"
sudo apt update
sudo apt install -y \
    build-essential cmake pkg-config git \
    qt6-base-dev \
    libgtest-dev \
    libasound2-dev libpulse-dev \
    libx11-dev libxext-dev libxrandr-dev libxcursor-dev \
    libxfixes-dev libxi-dev libxss-dev libxtst-dev \
    libxkbcommon-dev libdrm-dev libgbm-dev \
    libgl1-mesa-dev libegl1-mesa-dev \
    libwayland-dev wayland-protocols libdecor-0-dev \
    libpipewire-0.3-dev libudev-dev

# ── 2. Submodulos (SDL3, SDL3_image, SDL3_ttf, SDL3_mixer) ───────────────────
step "Inicializando submodulos de git (SDL vendoreado)"
git -C "${REPO_DIR}" submodule update --init --recursive

# ── 3. Compilar ──────────────────────────────────────────────────────────────
step "Configurando y compilando (CMake, Release)"
cmake -S "${REPO_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="${HOME}/.local"
cmake --build "${BUILD_DIR}" -j"$(nproc)"

# ── 4. Tests unitarios ───────────────────────────────────────────────────────
step "Corriendo tests unitarios"
ctest --test-dir "${BUILD_DIR}" --output-on-failure

# ── 5. Instalar ──────────────────────────────────────────────────────────────
step "Instalando en ~/.local y ~/.config"
cmake --install "${BUILD_DIR}"

echo
echo "Instalacion completa."
echo "  Binarios: ~/.local/bin/{client,server,map_editor}"
echo "  Assets:   ~/.local/share/argentum/"
echo "  Config:   ~/.config/argentum/config.toml"
echo
echo "Si ~/.local/bin no esta en tu PATH, agregalo con:"
echo '  echo '"'"'export PATH="$HOME/.local/bin:$PATH"'"'"' >> ~/.bashrc && source ~/.bashrc'
