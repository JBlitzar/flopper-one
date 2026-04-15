#pragma once

// TFT_eSPI setup (forced-in via platformio.ini build_flags)
// Display: ST7789, SPI, 240x240

#define ST7789_DRIVER

// Fonts
// Required for drawString/setTextSize usage.
#define LOAD_GLCD
// Optional extra built-in fonts (enable if you want them):
// #define LOAD_FONT2
// #define LOAD_FONT4
// #define LOAD_FONT6
// #define LOAD_FONT7
// #define LOAD_FONT8

#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// ESP32 SPI wiring
#define TFT_SCLK 19
#define TFT_MOSI 18
#define TFT_MISO -1

// Control pins
#define TFT_DC  32
#define TFT_CS  -1
#define TFT_RST 33

#define TFT_INVERSION_ON

#define SPI_FREQUENCY 4000000
