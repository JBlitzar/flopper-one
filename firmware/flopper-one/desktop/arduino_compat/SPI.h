#pragma once

#include <cstdint>
#include <cstddef>

class SPISettings
{
public:
    SPISettings(uint32_t, uint8_t, uint8_t) {}
};

class SPIClass
{
public:
    void begin() {}
    void end() {}
    void beginTransaction(const SPISettings &) {}
    void endTransaction() {}

    void setFrequency(uint32_t) {}

    uint8_t transfer(uint8_t data) { return data; }
    uint16_t transfer16(uint16_t data) { return data; }
    void transfer(uint8_t *, size_t) {}
};

inline SPIClass SPI;
