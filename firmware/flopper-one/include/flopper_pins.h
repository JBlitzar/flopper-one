#pragma once

#include <Arduino.h>

namespace flopper::pins
{

    // GPIO pins
    static constexpr uint8_t TSOP_OUT = 23; // TSOP IR receiver out
    static constexpr uint8_t DPAD_UP = 14;  // normal buttons
    static constexpr uint8_t DPAD_LEFT = 27;
    static constexpr uint8_t DPAD_RIGHT = 12;
    static constexpr uint8_t DPAD_DOWN = 26;
    static constexpr uint8_t DPAD_CENTER = 25;

    static constexpr uint8_t PN532_SDA = 17; // pn532 nfc/rfid thing
    static constexpr uint8_t PN532_SCL = 15;

    // Display (ST7789) is SPI, not I2C.
    static constexpr uint8_t DISP_SCL = 19; // SPI SCLK (as wired)
    static constexpr uint8_t DISP_SDA = 18; // SPI MOSI (as wired)
    static constexpr uint8_t DISP_DC = 32;  // SPI D/C (as wired)
    static constexpr int8_t DISP_CS = -1;   // no CS (tied active)
    static constexpr uint8_t DISP_RST = 33; // reset (as wired)

    // Preferred names
    static constexpr uint8_t DISP_SCLK = DISP_SCL;
    static constexpr uint8_t DISP_MOSI = DISP_SDA;

    static constexpr uint8_t IR_LED = 16;

}
