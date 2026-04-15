#pragma once

#include <cstddef>
#include <cstdint>

// Minimal TwoWire/Wire shim for desktop builds.
// Enough to satisfy headers; the desktop UI doesn't use I2C.

class TwoWire
{
public:
    void begin() {}
    void begin(int, int) {}

    void beginTransmission(uint8_t) {}
    uint8_t endTransmission() { return 0; }

    int requestFrom(uint8_t, uint8_t, bool = true) { return 0; }

    size_t write(uint8_t) { return 1; }
    int available() { return 0; }
    int read() { return -1; }
};

inline TwoWire Wire;
