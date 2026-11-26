#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

cd "$(dirname "$0")/../.."
source_dir=$(pwd -P)
build_dir="$source_dir/build/ci"
export_dir="$build_dir/exports"
dist="$source_dir/dist"
platform=${1:?expected linux-x64, macos-arm64 or windows-x64}
version=$(sed -nE 's/^set\(Launcher_RELEASE_VERSION "([0-9]+\.[0-9]+\.[0-9]+)" CACHE STRING.*$/\1/p' CMakeLists.txt)
test -n "$version"
mkdir -p "$dist" "$export_dir"

sha256() {
    if command -v sha256sum >/dev/null; then sha256sum "$@"; else shasum -a 256 "$@"; fi
}

fetch() {
    local url=$1 destination=$2 expected=$3 actual
    curl -fsSL --retry 3 -o "$destination" "$url"
    actual=$(sha256 "$destination" | awk '{print $1}')
    if [[ "$actual" != "$expected" ]]; then
        printf 'SHA-256 mismatch for %s\n' "$url" >&2
        exit 1
    fi
}

options=(
    -DCMAKE_BUILD_TYPE=Release
    -DENABLE_LTO=OFF
    -DBUILD_TESTING=ON
    "-DLauncher_BUILD_PLATFORM=sanctuary-$platform"
    -DLauncher_MSA_CLIENT_ID=8e8a2428-a727-4668-98ea-2fdaf0037943
    -DLauncher_SANCTUARY_UPDATE_KEY=er2KxCg8YIHTQaEZkViHuJJV/EFFlNeCyLzOWyra3Fs=
    -DLauncher_BUG_TRACKER_URL=https://github.com/sanctuary-minecraft/sanctuary-launcher/issues
)

case "$platform" in
    linux-x64)
        test "$(uname -s)-$(uname -m)" = Linux-x86_64
        jobs=$(nproc)
        options+=(-DLauncher_ENABLE_JAVA_DOWNLOADER=OFF)
        ;;
    macos-arm64)
        test "$(uname -s)-$(uname -m)" = Darwin-arm64
        jobs=$(sysctl -n hw.logicalcpu)
        options+=(
            -DCMAKE_OSX_DEPLOYMENT_TARGET=26.0
            -DCMAKE_OSX_ARCHITECTURES=arm64
            "-DCMAKE_PREFIX_PATH=$(brew --prefix);$(brew --prefix libarchive);$(brew --prefix zlib)"
            -DLauncher_ENABLE_JAVA_DOWNLOADER=ON
        )
        ;;
    windows-x64)
        test "${MSYSTEM:-}" = CLANG64
        export CC=clang CXX=clang++
        java_folder=$(cygpath -u "${JAVA_HOME:?set JAVA_HOME to JDK 17}")
        export PATH="$java_folder/bin:$PATH"
        export JAVA_HOME="$(cygpath -m "$java_folder")"
        jobs=$(nproc)
        fetch https://github.com/vslavik/winsparkle/releases/download/v0.9.4/WinSparkle-0.9.4.zip \
            "$build_dir/WinSparkle.zip" 6037df37fc263bd1650a1c4949681a9d40ffe991d01f35892a406cb5d103c976
        unzip -q "$build_dir/WinSparkle.zip" -d "$build_dir"
        export MSYS2_ARG_CONV_EXCL='*'
        options+=(
            -DCMAKE_CXX_FLAGS_RELEASE="-O2 -DNDEBUG"
            "-DLauncher_WINSPARKLE_DIR=$(cygpath -m "$build_dir/WinSparkle-0.9.4")"
            "-DJava_JAVA_EXECUTABLE=$JAVA_HOME/bin/java.exe"
            "-DJava_JAVAC_EXECUTABLE=$JAVA_HOME/bin/javac.exe"
            "-DJava_JAR_EXECUTABLE=$JAVA_HOME/bin/jar.exe"
            -DLauncher_ENABLE_JAVA_DOWNLOADER=ON
        )
        source_dir=$(cygpath -m "$source_dir")
        build_dir=$(cygpath -m "$build_dir")
        ;;
    *) printf 'Unknown platform: %s\n' "$platform" >&2; exit 2 ;;
esac

cmake -S "$source_dir" -B "$build_dir" -G Ninja "${options[@]}"
cmake --build "$build_dir" --parallel "$jobs"
QT_QPA_PLATFORM=offscreen ctest --test-dir "$build_dir" --output-on-failure --no-tests=error

archive="Sanctuary-Launcher-$platform.zip"
case "$platform" in
    linux-x64)
        tools_folder="$build_dir/packaging-tools"
        mkdir -p "$tools_folder"
        fetch https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20251107-1/linuxdeploy-x86_64.AppImage \
            "$tools_folder/linuxdeploy-x86_64.AppImage" c20cd71e3a4e3b80c3483cef793cda3f4e990aca14014d23c544ca3ce1270b4d
        fetch https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20250213-1/linuxdeploy-plugin-qt-x86_64.AppImage \
            "$tools_folder/linuxdeploy-plugin-qt-x86_64.AppImage" 15106be885c1c48a021198e7e1e9a48ce9d02a86dd0a1848f00bdbf3c1c92724
        fetch https://github.com/AppImage/appimagetool/releases/download/1.9.1/appimagetool-x86_64.AppImage \
            "$tools_folder/appimagetool-x86_64.AppImage" ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0
        chmod +x "$tools_folder/"*.AppImage
        bash scripts/export-sanctuary-linux.sh "$build_dir" "$tools_folder" "$export_dir"
        mv "$export_dir/Sanctuary-Launcher-$platform-preview.zip" "$dist/$archive"
        ;;
    macos-arm64)
        bash scripts/export-sanctuary-macos.sh "$build_dir" "$export_dir"
        mv "$export_dir/Sanctuary-Launcher-$platform-preview.zip" "$dist/$archive"
        ;;
    windows-x64)
        stage="$build_dir/install"
        cmake --install "$build_dir" --config Release --prefix "$stage"
        cp LICENSE COPYING.md "$export_dir/"
        makensis "-DVERSION=$version" "-DPAYLOAD=$(cygpath -w "$stage")" \
            "-DOUTPUT=$(cygpath -w "$export_dir/Sanctuary-Launcher-windows-x64.exe")" \
            "$(cygpath -w program_info/sanctuary_install.nsi)"
        (cd "$export_dir" && zip -q "$dist/$archive" Sanctuary-Launcher-windows-x64.exe LICENSE COPYING.md)
        ;;
esac
unzip -tq "$dist/$archive"
(cd "$dist" && sha256 "$archive" | tee "$archive.sha256")
