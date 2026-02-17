#pragma once

// TFT_eSPI setup (forced-in via platformio.ini build_flags)
// Display: ST7789, SPI, 240x240

#define ST7789_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// ESP32 SPI wiring
#define TFT_SCLK 18
#define TFT_MOSI 19
#define TFT_MISO -1

// Control pins
#define TFT_DC  32
#define TFT_CS  -1
#define TFT_RST 33

#define SPI_FREQUENCY 40000000
