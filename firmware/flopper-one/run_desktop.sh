#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT/desktop/build"
BIN="$BUILD_DIR/flopper_desktop"

TFT_LIB="$ROOT/.pio/libdeps/esp32dev/TFT_eSPI"

mkdir -p "$BUILD_DIR"

if [[ ! -d "$TFT_LIB" ]]; then
  echo "TFT_eSPI not found under .pio; running PlatformIO build once to fetch deps..."
  pio run -e esp32dev >/dev/null
fi

clang++ \
  -std=c++17 \
  -O2 \
  -DFLOPPER_DESKTOP=1 \
  -DUSER_SETUP_LOADED=1 \
  -include "$ROOT/include/tft_espi_setup.h" \
  -I"$ROOT/desktop/arduino_compat" \
  -I"$ROOT/include" \
  -I"$ROOT/desktop" \
  -I"$TFT_LIB" \
  "$ROOT/desktop/main.cpp" \
  "$ROOT/desktop/cocoa_bridge.mm" \
  "$TFT_LIB/TFT_eSPI.cpp" \
  -framework Cocoa \
  -o "$BIN"

if [[ "${1:-}" == "--build-only" ]]; then
  echo "Built: $BIN"
  exit 0
fi

exec "$BIN"
