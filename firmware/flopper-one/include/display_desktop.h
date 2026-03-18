#pragma once

// Desktop display shim that uses the real TFT_eSPI text/graphics code to draw into
// a 240x240 RGB565 framebuffer (via TFT_eSprite), then presents it in a Cocoa window.

#include "../desktop/cocoa_bridge.h"

#include <TFT_eSPI.h>

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
            ensure_sprite_();
            sprite_.fillSprite(color);
        }

        void draw_text(int x, int y, const char *str, uint32_t color, int size)
        {
            ensure_sprite_();
            sprite_.setTextColor((uint16_t)color, TFT_BLACK, true);
            sprite_.setTextSize((uint8_t)size);
            sprite_.drawString(str ? str : "", x, y);
        }

        void draw_text(int x, int y, const char *str, uint32_t color, int size, uint32_t highlight)
        {
            ensure_sprite_();
            sprite_.setTextColor((uint16_t)color, (uint16_t)highlight, true);
            sprite_.setTextSize((uint8_t)size);
            sprite_.drawString(str ? str : "", x, y);
        }

        void draw_rect(int x, int y, int w, int h, uint32_t color, int r = 0)
        {
            ensure_sprite_();
            if (r > 0)
                sprite_.drawRoundRect(x, y, w, h, r, (uint16_t)color);
            else
                sprite_.drawRect(x, y, w, h, (uint16_t)color);
        }

        void fill_rect(int x, int y, int w, int h, uint32_t color, int r = 0)
        {
            ensure_sprite_();
            if (r > 0)
                sprite_.fillRoundRect(x, y, w, h, r, (uint16_t)color);
            else
                sprite_.fillRect(x, y, w, h, (uint16_t)color);
        }

        void present()
        {
            ensure_sprite_();
            auto *p = (uint16_t *)sprite_.getPointer();
            flopper::desktop::render_rgb565(p, pixel_w_, pixel_h_);
        }

        void set_geometry(int16_t width_px, int16_t height_px, int16_t, int)
        {
            pixel_w_ = width_px;
            pixel_h_ = height_px;
        }

        void init_window(double scale = 1.0)
        {
            flopper::desktop::init_window(pixel_w_, pixel_h_, scale);
        }

    private:
        int16_t pixel_w_ = 240;
        int16_t pixel_h_ = 240;

        TFT_eSPI tft_{};
        TFT_eSprite sprite_{&tft_};
        bool sprite_ready_ = false;

        Display() = default;

        void ensure_sprite_()
        {
            if (sprite_ready_)
                return;

            tft_.init();
            tft_.setRotation(0);

            sprite_.setColorDepth(16);
            sprite_.createSprite(pixel_w_, pixel_h_);
            sprite_.fillSprite(TFT_BLACK);
            sprite_ready_ = true;
        }
    };
}
