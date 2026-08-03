#!/bin/bash
set -e

CHIAKI_SRC="/tmp/chiaki-src"
BUILD_DIR="/tmp/chiaki-build"
OUTPUT_DIR="/output"

mkdir -p "$BUILD_DIR" "$OUTPUT_DIR"

# Copy local source and fill in git submodules
echo "=== Preparing Chiaki source with submodules ==="
if [ ! -d "$CHIAKI_SRC" ]; then
    cp -r /chiaki-local "$CHIAKI_SRC"

    # nanopb submodule (v0.4.7 compatible)
    echo "--- Fetching nanopb ---"
    git clone --depth 1 https://github.com/nanopb/nanopb.git "$CHIAKI_SRC/third-party/nanopb"

    # jerasure and gf-complete (thestr4ng3r forks)
    echo "--- Fetching jerasure ---"
    git clone --depth 1 https://git.sr.ht/~thestr4ng3r/jerasure "$CHIAKI_SRC/third-party/jerasure"

    echo "--- Fetching gf-complete ---"
    git clone --depth 1 https://git.sr.ht/~thestr4ng3r/gf-complete "$CHIAKI_SRC/third-party/gf-complete"
fi

# Find host Qt tools (moc, rcc, uic must run on host arch)
HOST_QT_TOOLS=$(dirname $(which moc 2>/dev/null || find /usr -name moc -not -path "*/arm*" 2>/dev/null | head -1))
echo "Host Qt tools: $HOST_QT_TOOLS"

# Cross-compile toolchain settings
export CC=aarch64-linux-gnu-gcc
export CXX=aarch64-linux-gnu-g++
export AR=aarch64-linux-gnu-ar
export STRIP=aarch64-linux-gnu-strip

# pkg-config wrapper for cross-compilation
cat > /usr/local/bin/aarch64-pkg-config << 'PKGEOF'
#!/bin/sh
export PKG_CONFIG_DIR=
export PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=/
exec pkg-config "$@"
PKGEOF
chmod +x /usr/local/bin/aarch64-pkg-config

cmake -S "$CHIAKI_SRC" -B "$BUILD_DIR" \
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

cmake --build "$BUILD_DIR" -j$(nproc)

echo "--- Build complete ---"
echo "Binaries:"
find "$BUILD_DIR" -maxdepth 3 -type f -executable | xargs file 2>/dev/null | grep -i aarch64 | head -20

# Copy output
cp "$BUILD_DIR/gui/chiaki" "$OUTPUT_DIR/" 2>/dev/null || true
cp "$BUILD_DIR/cli/chiaki-cli" "$OUTPUT_DIR/" 2>/dev/null || true
ls -lh "$OUTPUT_DIR"/
