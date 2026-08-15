#!/usr/bin/env bash
set -euo pipefail

cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null || exit
cd ..

APPDIR="AppDir"
BUILD_DIR="build/linux-release"

rm -rf "$APPDIR"
mkdir -p "$APPDIR"
mkdir -p "$APPDIR/usr/share/metainfo"

cmake --preset linux-release -DCMAKE_INSTALL_PREFIX=/usr/local -DMZ_BZIP2=OFF
cmake --build "$BUILD_DIR" --parallel $(nproc)
strip "$BUILD_DIR/monitor-maker"

DESTDIR="$APPDIR" cmake --install "$BUILD_DIR"
rDNS="io.github.this_is_not_available.portal_2_monitor_maker"
METAINFO="$APPDIR/usr/share/metainfo/$rDNS.metainfo.xml"
APPDATA="$APPDIR/usr/share/metainfo/$rDNS.appdata.xml"

cp assets/monitor-maker.desktop "$APPDIR/$rDNS.desktop"
cp assets/appstream.metainfo.xml "$METAINFO"
cp assets/appstream.metainfo.xml "$APPDATA"
cp assets/MonMakeLogo_512.png "$APPDIR/MonMakeLogo_512.png"

if [ ! -f linuxdeploy.AppImage ]; then
    wget -O linuxdeploy.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy.AppImage
fi

if [ -n "${DEPLOY_APPIMAGE_FILENAME:-}" ]; then
    export LDAI_OUTPUT="$DEPLOY_APPIMAGE_FILENAME"
fi
./linuxdeploy.AppImage --appimage-extract
./squashfs-root/AppRun --appdir "$APPDIR" \
  --desktop-file="$APPDIR/$rDNS.desktop" \
  --icon-file="$APPDIR/MonMakeLogo_512.png" \
  --executable "$APPDIR/usr/local/bin/monitor-maker" \
  --output appimage
