#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT/desktop/build"
BIN="$BUILD_DIR/flopper_desktop"

GFX_LIB="$ROOT/.pio/libdeps/esp32dev/Adafruit GFX Library"
BUSIO_LIB="$ROOT/.pio/libdeps/esp32dev/Adafruit BusIO"

mkdir -p "$BUILD_DIR"
if [[ ! -d "$GFX_LIB" || ! -d "$BUSIO_LIB" ]]; then
  echo "Adafruit_GFX not found under .pio; running PlatformIO build once to fetch deps..."
  pio run -e esp32dev >/dev/null
fi

clang++ \
  -std=c++17 \
  -O2 \
  -DFLOPPER_DESKTOP=1 \
  -DARDUINO=100 \
  -I"$ROOT/desktop/arduino_compat" \
  -I"$ROOT/include" \
  -I"$ROOT/desktop" \
  -I"$GFX_LIB" \
  -I"$BUSIO_LIB" \
  "$ROOT/desktop/main.cpp" \
  "$ROOT/desktop/cocoa_bridge.mm" \
  "$GFX_LIB/Adafruit_GFX.cpp" \
  "$GFX_LIB/glcdfont.c" \
  -framework Cocoa \
  -o "$BIN"

if [[ "${1:-}" == "--build-only" ]]; then
  echo "Built: $BIN"
  exit 0
fi

exec "$BIN"
