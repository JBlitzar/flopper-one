#pragma once

#include <cstdint>

namespace flopper::colors
{
    // RGB565 constants (uint16_t). Keep these centralized so the rest of the codebase
    // doesn't depend on a specific display driver library's `TFT_*` defines.

    inline constexpr uint16_t BLACK = 0x0000;
    inline constexpr uint16_t WHITE = 0xFFFF;

    inline constexpr uint16_t RED = 0xF800;
    inline constexpr uint16_t GREEN = 0x07E0;
    inline constexpr uint16_t CYAN = 0x07FF;

    // Common greys (RGB565)
    inline constexpr uint16_t DARKGREY = 0x7BEF;
    inline constexpr uint16_t LIGHTGREY = 0xC618;
}
