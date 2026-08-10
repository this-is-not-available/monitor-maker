#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null || exit
cd ..

APPDIR="AppDir"
BUILD_DIR="build/linux-release"

rm -rf "$APPDIR"
mkdir -p "$APPDIR"

cmake --preset linux-release
cmake --build "$BUILD_DIR" --parallel $(nproc)
strip "$BUILD_DIR/monitor-maker"

DESTDIR="$APPDIR" cmake --install "$BUILD_DIR"

cp assets/monitor-maker.desktop "$APPDIR/monitor-maker.desktop"
cp assets/MonMakeLogo_512.png "$APPDIR/MonMakeLogo_512.png"

if [ ! -f linuxdeploy.AppImage ]; then
    wget -O linuxdeploy.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy.AppImage
fi

#export OUTPUT="MyApp-x86_64.AppImage"
./linuxdeploy.AppImage --appdir "$APPDIR" \
  --desktop-file="$APPDIR/monitor-maker.desktop" \
  --icon-file="$APPDIR/MonMakeLogo_512.png" \
  --executable "$APPDIR/usr/bin/monitor-maker" \
  --output appimage
