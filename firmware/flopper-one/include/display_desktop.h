#pragma once

// Desktop display shim that draws into a 240x240 RGB565 framebuffer (via Adafruit_GFX
// GFXcanvas16), then presents it in a Cocoa window.

// NOTE: This implementation uses Adafruit_GFX's GFXcanvas16 as the backing store,
// so the desktop UI stays consistent with the on-device Adafruit_GFX rendering.

#include "../desktop/cocoa_bridge.h"

#include "colors.h"

#include <Adafruit_GFX.h>

#include <cstdint>
#include <memory>

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
            ensure_canvas_();
            canvas_->fillScreen((uint16_t)color);
        }

        void draw_text(int x, int y, const char *str, uint32_t color, int size)
        {
            ensure_canvas_();
            canvas_->setCursor(x, y);
            canvas_->setTextColor((uint16_t)color, flopper::colors::BLACK);
            canvas_->setTextSize((uint8_t)size);
            canvas_->print(str ? str : "");
        }

        void draw_text(int x, int y, const char *str, uint32_t color, int size, uint32_t highlight)
        {
            ensure_canvas_();
            canvas_->setCursor(x, y);
            canvas_->setTextColor((uint16_t)color, (uint16_t)highlight);
            canvas_->setTextSize((uint8_t)size);
            canvas_->print(str ? str : "");
        }

        void draw_rect(int x, int y, int w, int h, uint32_t color, int r = 0)
        {
            ensure_canvas_();
            if (r > 0)
                canvas_->drawRoundRect(x, y, w, h, r, (uint16_t)color);
            else
                canvas_->drawRect(x, y, w, h, (uint16_t)color);
        }

        void fill_rect(int x, int y, int w, int h, uint32_t color, int r = 0)
        {
            ensure_canvas_();
            if (r > 0)
                canvas_->fillRoundRect(x, y, w, h, r, (uint16_t)color);
            else
                canvas_->fillRect(x, y, w, h, (uint16_t)color);
        }

        void present()
        {
            ensure_canvas_();
            flopper::desktop::render_rgb565(canvas_->getBuffer(), pixel_w_, pixel_h_);
        }

        void set_geometry(int16_t width_px, int16_t height_px, int16_t, int)
        {
            pixel_w_ = width_px;
            pixel_h_ = height_px;
            canvas_.reset();
        }

        void init_window(double scale = 1.0)
        {
            flopper::desktop::init_window(pixel_w_, pixel_h_, scale);
        }

    private:
        int16_t pixel_w_ = 240;
        int16_t pixel_h_ = 240;

        std::unique_ptr<GFXcanvas16> canvas_;

        Display() = default;

        void ensure_canvas_()
        {
            if (canvas_)
                return;

            canvas_ = std::make_unique<GFXcanvas16>(pixel_w_, pixel_h_);
            canvas_->setTextWrap(false);
            canvas_->fillScreen(flopper::colors::BLACK);
        }
    };
}
