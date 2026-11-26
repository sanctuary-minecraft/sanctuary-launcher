#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# Package a tested x86_64 build using linuxdeploy, its Qt plugin and appimagetool.
set -euo pipefail

if [[ $# != 3 || $(uname -s) != Linux || $(uname -m) != x86_64 ]]; then
    printf 'Usage on Linux x86_64: %s <cmake-build-dir> <packaging-tools-dir> <output-dir>\n' "$0" >&2
    exit 2
fi

source_dir=$(cd "$(dirname "$0")/.." && pwd)
build_dir=$(realpath "$1")
tools_dir=$(realpath "$2")
mkdir -p "$3"
output_dir=$(realpath "$3")
stage=$(mktemp -d "$output_dir/linux-stage.XXXXXX")
appdir="$stage/SanctuaryLauncher.AppDir"
cmake --install "$build_dir" --config Release --prefix "$appdir/usr"

app_id=net.sanctuary-minecraft.launcher
icon="usr/share/icons/hicolor/256x256/apps/$app_id.png"
export APPIMAGE_EXTRACT_AND_RUN=1
export QMAKE="${QMAKE:-qmake6}"
export EXTRA_QT_MODULES='svg;webenginecore'
export QML_SOURCES_PATHS="$source_dir/launcher/resources/sanctuary"
export EXTRA_PLATFORM_PLUGINS=libqoffscreen.so
export NO_STRIP=1
export PATH="$tools_dir:$PATH"
extra_libraries=()
webview_plugin=$("$QMAKE" -query QT_INSTALL_PLUGINS)/webview/libqtwebview_webengine.so
install -D -m 0755 "$webview_plugin" "$appdir/usr/plugins/webview/libqtwebview_webengine.so"
extra_libraries+=(--library "$webview_plugin")
if ldd "$appdir/usr/bin/sanctuary-launcher" | awk '/libqt6keychain/ {found=1} END {exit !found}'; then
    secret_library=$(ldconfig -p | awk '/libsecret-1\.so\.0/ && !path {path=$NF} END {print path}')
    if [[ -z "$secret_library" ]]; then
        printf 'libsecret-1.so.0 is required to package the desktop keyring backend.\n' >&2
        exit 1
    fi
    extra_libraries+=(--library "$secret_library")
fi
"$tools_dir/linuxdeploy-x86_64.AppImage" --appdir "$appdir" \
    --executable "$appdir/usr/bin/sanctuary-launcher" \
    --desktop-file "$appdir/usr/share/applications/$app_id.desktop" \
    --icon-file "$appdir/$icon" --plugin qt "${extra_libraries[@]}"
ln -sf "$icon" "$appdir/$app_id.png"
ln -sf "$app_id.png" "$appdir/.DirIcon"

appimage=Sanctuary-Launcher-x86_64.AppImage
ARCH=x86_64 "$tools_dir/appimagetool-x86_64.AppImage" "$appdir" "$stage/$appimage"
cp "$source_dir/LICENSE" "$source_dir/COPYING.md" "$stage"
archive=Sanctuary-Launcher-linux-x64-preview.zip
(cd "$stage" && zip -q "$archive" "$appimage" LICENSE COPYING.md)
mv "$stage/$archive" "$output_dir/$archive"
(cd "$output_dir" && sha256sum "$archive" > "$archive.sha256" && cat "$archive.sha256")
