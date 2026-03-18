#pragma once

#include <cstdint>

namespace flopper::desktop
{
    void init_window(int pixel_w, int pixel_h, double scale);
    bool pump_events();

    // Input queue used by InputDispatcher.
    bool next_input_event(int &evt_out); // returns false if none

    // Render framebuffer in RGB565 (little-endian uint16_t per pixel).
    void render_rgb565(const uint16_t *pixels, int w, int h);
}
