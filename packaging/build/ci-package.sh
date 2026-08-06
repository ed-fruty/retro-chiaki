#!/bin/bash
# Builds Retro Chiaki for aarch64 and assembles the release zip, run by
# .github/workflows/release.yml on a native ubuntu-22.04 GitHub Actions
# runner (not inside Docker -- the runner already IS the same base OS as
# packaging/build/Dockerfile, so the same apt packages apply directly).
#
# Unlike packaging/build/build.sh (the local dev helper, which runs inside
# the Docker image and fetches third-party/{nanopb,jerasure,gf-complete}
# itself because a bare local checkout may not have those submodules
# initialized), this script assumes actions/checkout already populated
# submodules via `submodules: recursive` and does not touch them.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="/tmp/chiaki-build"
STAGE_DIR="/tmp/chiaki-release"
VERSION="${1:?usage: ci-package.sh vX.Y.Z}"
ZIP_NAME="retro-chiaki-${VERSION}-portmaster-muos-h700.zip"

export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export AR=aarch64-linux-gnu-ar
export STRIP=aarch64-linux-gnu-strip

# pkg-config wrapper for cross-compilation (Ubuntu doesn't ship an
# aarch64-linux-gnu-pkg-config binary by default).
sudo tee /usr/local/bin/aarch64-pkg-config > /dev/null << 'PKGEOF'
#!/bin/sh
export PKG_CONFIG_DIR=
export PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=/
exec pkg-config "$@"
PKGEOF
sudo chmod +x /usr/local/bin/aarch64-pkg-config

echo "=== Configuring ==="
cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
    -DCMAKE_FIND_ROOT_PATH="/usr/lib/aarch64-linux-gnu;/usr/aarch64-linux-gnu;/usr" \
    -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
    -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
    -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY \
    -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake \
    -DCMAKE_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu \
    -DCMAKE_INCLUDE_PATH="/usr/include;/usr/include/aarch64-linux-gnu" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCHIAKI_ENABLE_GUI=ON \
    -DCHIAKI_ENABLE_CLI=ON \
    -DCHIAKI_ENABLE_TESTS=OFF \
    -DCHIAKI_ENABLE_FFMPEG_DECODER=ON \
    -DCHIAKI_LIB_ENABLE_OPUS=ON \
    -DCHIAKI_GUI_ENABLE_SDL_GAMECONTROLLER=ON \
    -DCHIAKI_USE_SYSTEM_NANOPB=OFF \
    -DCHIAKI_USE_SYSTEM_JERASURE=OFF \
    -DPKG_CONFIG_EXECUTABLE=/usr/local/bin/aarch64-pkg-config \
    -DOpus_INCLUDE_DIRS=/usr/include \
    -DOpus_LIBRARIES=/usr/lib/aarch64-linux-gnu/libopus.so \
    -G Ninja

echo "=== Building ==="
cmake --build "$BUILD_DIR" -j"$(nproc)"

CHIAKI_BIN="$BUILD_DIR/gui/chiaki"
CHIAKI_CLI_BIN="$BUILD_DIR/cli/chiaki-cli"
file "$CHIAKI_BIN" "$CHIAKI_CLI_BIN"

echo "=== Building Mali EGL shim ==="
aarch64-linux-gnu-gcc -shared -fPIC -O0 -g \
    -o "$BUILD_DIR/libmaliegl.so" "$REPO_ROOT/packaging/build/mali_egl_shim.c" -ldl

# The Qt platform/image plugins below are loaded via dlopen at runtime, not
# recorded as ELF NEEDED entries on the main binaries, so they can't be
# discovered by dependency resolution -- they have to be listed explicitly.
# This is the same fixed set the last several manually-built releases shipped.
QT_PLUGIN_DIR="/usr/lib/aarch64-linux-gnu/qt5/plugins"
PLATFORM_PLUGINS=(libqeglfs.so libqminimalegl.so libqoffscreen.so)
IMAGEFORMAT_PLUGINS=(libqico.so libqjpeg.so libqsvg.so libqgif.so)

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR/ROMS/Ports" \
         "$STAGE_DIR/ports/chiaki/libs/qt5/plugins/platforms" \
         "$STAGE_DIR/ports/chiaki/libs/qt5/plugins/imageformats" \
         "$STAGE_DIR/ports/chiaki/xkb" \
         "$STAGE_DIR/MUOS/info/catalogue/External - Ports/box" \
         "$STAGE_DIR/MUOS/info/catalogue/External - Ports/preview"

echo "=== Resolving runtime library dependencies ==="
# Some transitively-linked libraries must come from the TARGET DEVICE, never
# be bundled, because they're tightly version-coupled to muOS's own base
# system and this Ubuntu 22.04 build environment's copy can be incompatible:
#   - libpulse.so.0: its private libpulsecommon-X.Y.so companion must be the
#     exact same build. muOS ships PulseAudio 13.99; this build environment's
#     libpulse-dev pulls in 15.99. The device already has a matched
#     libpulse.so.0/libpulsecommon-13.99.so pair at /usr/lib, which the
#     dynamic linker falls back to by default once ours is excluded here.
#   - glibc's own runtime (libc/libm/libpthread/etc.): this build environment
#     is Ubuntu 22.04 (glibc 2.35), muOS is glibc 2.38. glibc is forward-
#     compatible (older binaries run fine on newer glibc) but NOT the other
#     way around, and LD_LIBRARY_PATH would make our older copy shadow the
#     device's -- breaking any other system library (e.g. muOS's own
#     libsndfile.so.1) that needs a GLIBC_2.36+ symbol version. Always let
#     these resolve to the device's own copy.
#   - libevdev.so.2 / libudev.so.1: read the device's own evdev/uinput nodes
#     (including the virtual pointer gptokeyb creates for the connect UI's
#     mouse-click navigation), so they need to match muOS's own kernel/udev
#     setup, not this build environment's. Confirmed by direct A/B testing on
#     device: bundling this build's copies (which LD_LIBRARY_PATH makes take
#     priority over the device's) silently breaks all pre-stream button/click
#     input, since Qt's eglfs input handling loads successfully either way
#     but the bundled copies don't work with this device's uinput/evdev
#     nodes. The device has its own compatible copies at /usr/lib and /lib.
DO_NOT_BUNDLE=(
    libpulse.so.0
    libc.so.6
    libm.so.6
    libpthread.so.0
    libdl.so.2
    librt.so.1
    libresolv.so.2
    libnsl.so.1
    libutil.so.1
    libanl.so.1
    libevdev.so.2
    libudev.so.1
)

# Recursively follows each binary's ELF NEEDED entries against the aarch64
# system library paths and prints "resolved_path|soname" for every unique
# library reached. This bundles exactly what the dynamic linker will
# actually look for at runtime (nothing more), rather than a hand-picked or
# broadly-swept set of files.
resolve_deps() {
    local -a search_dirs=(/usr/lib/aarch64-linux-gnu /lib/aarch64-linux-gnu)
    local -A seen=()
    local -a queue=("$@")

    while [ "${#queue[@]}" -gt 0 ]; do
        local current="${queue[0]}"
        queue=("${queue[@]:1}")

        local needed
        needed="$(aarch64-linux-gnu-objdump -p "$current" 2>/dev/null | awk '/NEEDED/{print $2}')"
        local lib
        for lib in $needed; do
            [ -n "${seen[$lib]:-}" ] && continue
            seen[$lib]=1

            local skip=""
            local excluded
            for excluded in "${DO_NOT_BUNDLE[@]}"; do
                [ "$lib" = "$excluded" ] && skip=1 && break
            done
            [ -n "$skip" ] && continue

            local found=""
            local d
            for d in "${search_dirs[@]}"; do
                if [ -e "$d/$lib" ]; then
                    found="$d/$lib"
                    break
                fi
            done
            if [ -z "$found" ]; then
                echo "::warning::could not resolve runtime dependency '$lib' (needed by $current)" >&2
                continue
            fi

            echo "$found|$lib"
            queue+=("$found")
        done
    done
}

LIBS_DIR="$STAGE_DIR/ports/chiaki/libs"
declare -A copied=()
while IFS='|' read -r src soname; do
    [ -n "${copied[$soname]:-}" ] && continue
    copied[$soname]=1
    cp -L "$src" "$LIBS_DIR/$soname"
done < <(
    resolve_deps "$CHIAKI_BIN" "$CHIAKI_CLI_BIN"
    for p in "${PLATFORM_PLUGINS[@]}"; do
        resolve_deps "$QT_PLUGIN_DIR/platforms/$p"
    done
    for p in "${IMAGEFORMAT_PLUGINS[@]}"; do
        resolve_deps "$QT_PLUGIN_DIR/imageformats/$p"
    done
)
echo "Bundled ${#copied[@]} runtime libraries"

echo "=== Copying Qt plugins, Mali shim, XKB data ==="
for p in "${PLATFORM_PLUGINS[@]}"; do
    cp -L "$QT_PLUGIN_DIR/platforms/$p" "$LIBS_DIR/qt5/plugins/platforms/$p"
done
for p in "${IMAGEFORMAT_PLUGINS[@]}"; do
    cp -L "$QT_PLUGIN_DIR/imageformats/$p" "$LIBS_DIR/qt5/plugins/imageformats/$p"
done
cp "$BUILD_DIR/libmaliegl.so" "$LIBS_DIR/libmaliegl.so"
cp -r /usr/share/X11/xkb/. "$STAGE_DIR/ports/chiaki/xkb/"

echo "=== Copying binaries and static portmaster assets ==="
cp "$CHIAKI_BIN" "$STAGE_DIR/ports/chiaki/chiaki"
cp "$CHIAKI_CLI_BIN" "$STAGE_DIR/ports/chiaki/chiaki-cli"
chmod +x "$STAGE_DIR/ports/chiaki/chiaki" "$STAGE_DIR/ports/chiaki/chiaki-cli"

PORTMASTER_DIR="$REPO_ROOT/packaging/portmaster"
cp "$PORTMASTER_DIR/ports/chiaki/chiaki.gptk" "$STAGE_DIR/ports/chiaki/chiaki.gptk"
cp "$PORTMASTER_DIR/ROMS/Ports/Chiaki.sh" "$STAGE_DIR/ROMS/Ports/Chiaki.sh"
chmod +x "$STAGE_DIR/ROMS/Ports/Chiaki.sh"
cp "$PORTMASTER_DIR/MUOS/info/catalogue/External - Ports/box/Chiaki.png" \
   "$STAGE_DIR/MUOS/info/catalogue/External - Ports/box/Chiaki.png"
cp "$PORTMASTER_DIR/MUOS/info/catalogue/External - Ports/preview/Chiaki.png" \
   "$STAGE_DIR/MUOS/info/catalogue/External - Ports/preview/Chiaki.png"

# Deliberately NOT included: packaging/muos-launcher/ (the native muOS
# Application shortcut). It ships as its own separate, optional download,
# not folded into the main PortMaster release zip.

echo "=== Zipping ==="
mkdir -p "$REPO_ROOT/dist"
OUT_ZIP="$REPO_ROOT/dist/$ZIP_NAME"
rm -f "$OUT_ZIP"
( cd "$STAGE_DIR" && zip -qr "$OUT_ZIP" ROMS ports MUOS -x '*.DS_Store' )

echo "=== Done: $OUT_ZIP ==="
ls -lh "$OUT_ZIP"
