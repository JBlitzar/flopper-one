#pragma once
#include "flopper_pins.h"
#include "colors.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <string>

namespace flopper
{

    class Display
    {
    public:
        static Display &get_instance()
        {
            static Display instance;
            return instance;
        }

        void clear(uint32_t color)
        {
            tft_.fillScreen((uint16_t)color);
        }

        void draw_text(int x, int y, const char *str, uint32_t color, int size)
        {
            tft_.setCursor(x, y);
            tft_.setTextSize(size);
            tft_.setTextColor((uint16_t)color, flopper::colors::BLACK);
            tft_.print(str ? str : "");
        }

        void draw_text(int x, int y, const char *str, uint32_t color, int size, uint32_t highlight)
        {
            tft_.setCursor(x, y);
            tft_.setTextSize(size);
            tft_.setTextColor((uint16_t)color, (uint16_t)highlight);
            tft_.print(str ? str : "");
        }

        void draw_rect(int x, int y, int w, int h, uint32_t color, int r = 0)
        {
            if (r > 0)
                tft_.drawRoundRect(x, y, w, h, r, (uint16_t)color);
            else
                tft_.drawRect(x, y, w, h, (uint16_t)color);
        }
        void fill_rect(int x, int y, int w, int h, uint32_t color, int r = 0)
        {
            if (r > 0)
                tft_.fillRoundRect(x, y, w, h, r, (uint16_t)color);
            else
                tft_.fillRect(x, y, w, h, (uint16_t)color);
        }

    private:
        static constexpr int16_t kWidth = 240;
        static constexpr int16_t kHeight = 240;
        static constexpr uint32_t kSpiHz = 40000000;

        SPIClass hspi_{HSPI};
        Adafruit_ST7789 tft_{&hspi_, flopper::pins::DISP_CS, flopper::pins::DISP_DC, flopper::pins::DISP_RST};

        Display()
        {
            hspi_.begin(flopper::pins::DISP_SCLK, -1, flopper::pins::DISP_MOSI, -1);

            tft_.init(kWidth, kHeight, SPI_MODE3);
            tft_.setRotation(1);
            tft_.setSPISpeed(kSpiHz);
            tft_.setTextWrap(false);
            tft_.fillScreen(flopper::colors::BLACK);
        };

        // copied + edited from google AI overview lol

        Display(const Display &) = delete;
        Display &operator=(const Display &) = delete;

        Display(Display &&) = delete;
        Display &operator=(Display &&) = delete;
    };
}