#pragma once
#include <tft_espi_setup.h>
#include "flopper_pins.h"
#include <TFT_eSPI.h>
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
            tft.fillScreen(color);
        }

        void draw_text(int x, int y, std::string &str, uint32_t color, int size)
        {
            tft.setTextColor(color, TFT_BLACK);
            tft.setTextSize(size);
            tft.drawString(str.c_str(), x, y);
        }

        void draw_text(int x, int y, std::string &str, uint32_t color, int size, uint32_t highlight)
        {
            tft.setTextColor(color, highlight, true);
            tft.setTextSize(size);
            tft.drawString(str.c_str(), x, y);
        }

        void fill_rect(int x, int y, int w, int h, uint32_t color)
        {
            tft.fillRect(x, y, w, h, color);
        }

        void fill_rect(int x, int y, int w, int h, uint32_t color, int r)
        {
            tft.fillRoundRect(x, y, w, h, r, color);
        }

    private:
        TFT_eSPI tft;
        Display()
        {

            tft = TFT_eSPI();
            tft.init();
            tft.setRotation(0);
            tft.fillScreen(TFT_BLACK);
        };

        // copied + edited from google AI overview lol

        Display(const Display &) = delete;
        Display &operator=(const Display &) = delete;

        Display(Display &&) = delete;
        Display &operator=(Display &&) = delete;
    };
}