#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
# Archive a tested Apple Silicon build for local verification.
set -euo pipefail

if [[ $# != 2 || $(uname -s) != Darwin || $(uname -m) != arm64 ]]; then
    printf 'Usage on Apple Silicon: %s <cmake-build-dir> <output-dir>\n' "$0" >&2
    exit 2
fi

source_dir=$(cd "$(dirname "$0")/.." && pwd)
build_dir=$(cd "$1" && pwd)
mkdir -p "$2"
output_dir=$(cd "$2" && pwd)
stage=$(mktemp -d "$output_dir/macos-stage.XXXXXX")
cmake --install "$build_dir" --config Release --prefix "$stage" > "$stage/install.log" 2>&1 || {
    tail -40 "$stage/install.log" >&2
    exit 1
}

app="$stage/sanctuary-launcher.app"
binary="$app/Contents/MacOS/sanctuary-launcher"
test "$(lipo -archs "$binary")" = arm64
for helper in JavaCheck NewLaunch NewLaunchLegacy; do
    cmp "$build_dir/jars/$helper.jar" "$app/Contents/Resources/jars/$helper.jar"
done
icon_name=$(/usr/libexec/PlistBuddy -c 'Print :CFBundleIconFile' "$app/Contents/Info.plist")
test "$icon_name" = SanctuaryLauncher.icns
cmp "$source_dir/program_info/sanctuary-launcher.icns" "$app/Contents/Resources/$icon_name"
test ! -e "$app/Contents/Resources/Assets.car"
test -f "$app/Contents/Resources/qtlogging.ini"
# CMake and macdeployqt can rewrite libraries after copying their original signatures.
for library in "$app"/Contents/Frameworks/*.dylib "$app"/Contents/PlugIns/*/*.dylib; do
    if [[ -f "$library" && ! -L "$library" ]] && ! codesign --verify "$library" 2>/dev/null; then
        codesign --force --sign - "$library"
    fi
done
codesign --force --sign - "$app"
codesign --verify --deep --strict "$app"
"$binary" --version

archive=Sanctuary-Launcher-macos-arm64-preview.zip
ditto -c -k --sequesterRsrc --keepParent "$app" "$stage/$archive"
cp "$source_dir/LICENSE" "$source_dir/COPYING.md" "$stage"
(cd "$stage" && zip -q "$archive" LICENSE COPYING.md)
if [[ -f "$build_dir/frameworks/Sparkle/LICENSE" ]]; then
    cp "$build_dir/frameworks/Sparkle/LICENSE" "$stage/Sparkle-LICENSE"
    (cd "$stage" && zip -q "$archive" Sparkle-LICENSE)
fi
mv "$stage/$archive" "$output_dir/$archive"
(cd "$output_dir" && shasum -a 256 "$archive" > "$archive.sha256" && cat "$archive.sha256")
