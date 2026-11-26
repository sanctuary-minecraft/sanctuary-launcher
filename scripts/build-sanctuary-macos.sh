#!/bin/sh
# Build the local Sanctuary fork without installing it over another launcher.
set -eu
sanctuary_source_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
sanctuary_java_prefix=$(brew --prefix openjdk@17)
sanctuary_brew_prefix=$(brew --prefix)
sanctuary_archive_prefix=$(brew --prefix libarchive)

cmake -S "$sanctuary_source_dir" -B "$sanctuary_source_dir/build/native" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$sanctuary_brew_prefix;$sanctuary_archive_prefix" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=26.0 \
  -DBUILD_TESTING=ON \
  -DLauncher_BUILD_PLATFORM=sanctuary-macos-arm64 \
  -DJava_JAVA_EXECUTABLE="$sanctuary_java_prefix/bin/java" \
  -DJava_JAVAC_EXECUTABLE="$sanctuary_java_prefix/bin/javac" \
  -DJava_JAR_EXECUTABLE="$sanctuary_java_prefix/bin/jar"
cmake --build "$sanctuary_source_dir/build/native" --parallel "${SANCTUARY_BUILD_JOBS:-4}"
sanctuary_bundle_resources="$sanctuary_source_dir/build/native/sanctuary-launcher.app/Contents/Resources"
mkdir -p "$sanctuary_bundle_resources"
cp "$sanctuary_source_dir/program_info/sanctuary-launcher.icns" "$sanctuary_bundle_resources/SanctuaryLauncher.icns"
# add_jar builds these beside the bundle; the runtime searches inside it.
sanctuary_bundle_jars="$sanctuary_bundle_resources/jars"
mkdir -p "$sanctuary_bundle_jars"
for sanctuary_helper in JavaCheck.jar NewLaunch.jar NewLaunchLegacy.jar; do
    cp "$sanctuary_source_dir/build/native/jars/$sanctuary_helper" "$sanctuary_bundle_jars/$sanctuary_helper"
    cmp "$sanctuary_source_dir/build/native/jars/$sanctuary_helper" "$sanctuary_bundle_jars/$sanctuary_helper"
done
QT_QPA_PLATFORM=offscreen ctest --test-dir "$sanctuary_source_dir/build/native" --output-on-failure
