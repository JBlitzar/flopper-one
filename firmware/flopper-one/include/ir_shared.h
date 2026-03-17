#pragma once

#include <Arduino.h>

namespace flopper::ir_shared
{
    inline String last_summary = "";
    inline uint16_t last_raw_us[300] = {0};
    inline size_t last_raw_len = 0;
    inline uint16_t last_raw_khz = 38;
}
