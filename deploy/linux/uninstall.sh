#!/usr/bin/env bash
set -euo pipefail

PREFIX="${PREFIX:-/usr/local}"

files=(
    "$PREFIX/bin/AutoPower"
    "$PREFIX/share/applications/autopower.desktop"
    "$PREFIX/share/metainfo/autopower.appdata.xml"
    "$PREFIX/share/icons/hicolor/256x256/apps/autopower.png"
)

for file in "${files[@]}"; do
    if [ -e "$file" ] || [ -L "$file" ]; then
        rm -f "$file"
        echo "Removed $file"
    fi
done

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache -f "$PREFIX/share/icons/hicolor" >/dev/null 2>&1 || true
fi

if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$PREFIX/share/applications" >/dev/null 2>&1 || true
fi

echo "AutoPower removed from $PREFIX"
