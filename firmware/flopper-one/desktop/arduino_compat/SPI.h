#pragma once

#include <cstdint>
#include <cstddef>

// Arduino SPI constants/types expected by Adafruit_BusIO.
enum BitOrder : uint8_t
{
    LSBFIRST = 0,
    MSBFIRST = 1,
};

#ifndef SPI_MODE0
#define SPI_MODE0 0x00
#endif
#ifndef SPI_MODE1
#define SPI_MODE1 0x01
#endif
#ifndef SPI_MODE2
#define SPI_MODE2 0x02
#endif
#ifndef SPI_MODE3
#define SPI_MODE3 0x03
#endif

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
