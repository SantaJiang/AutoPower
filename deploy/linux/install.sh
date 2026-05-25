#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
PREFIX="${PREFIX:-/usr/local}"

echo "Installing AutoPower from: $BUILD_DIR"
echo "Install prefix: $PREFIX"

cmake --build "$BUILD_DIR" --config Release --parallel
cmake --install "$BUILD_DIR" --prefix "$PREFIX"

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f "$PREFIX/share/icons/hicolor" >/dev/null 2>&1 || true
fi

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$PREFIX/share/applications" >/dev/null 2>&1 || true
fi

echo "AutoPower installed. Run: $PREFIX/bin/AutoPower"
