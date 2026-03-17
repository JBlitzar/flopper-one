#pragma once

#include "../../app.h"
#include "../../display.h"
#include "../../ui.h"
#include "../../log.h"

#include "pn532_common.h"

#include <vector>

namespace flopper
{
    class Pn532InfoApp : public App
    {
    public:
        Pn532InfoApp() : App("PN532 Info") {}

        void on_enter() override
        {
            ok_ = pn532::ensure_init();
            version_ = ok_ ? pn532::dev.getFirmwareVersion() : 0;
            p3_ = ok_ ? pn532::dev.readGPIO() : 0;
            p3_out_ = p3_;
            selected_ = 0;
            last_poll_ms_ = 0;
            log::line("PN532", "info enter");
        }

        void on_exit() override
        {
            log::line("PN532", "info exit");
        }

        void tick() override
        {
            if (ok_ && millis() - last_poll_ms_ > 500)
            {
                p3_ = pn532::dev.readGPIO();
                last_poll_ms_ = millis();
            }
            draw();
        }

        void draw() override
        {
            flopper::ui::draw_status(Display::get_instance(), ok_ ? "PN532 Info" : "PN532 Info (not found)");
            Display::get_instance().fill_rect(0, 30, 240, 210, TFT_BLACK);

            if (!ok_)
            {
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, "No PN532 detected", TFT_WHITE, 1, TFT_BLACK);
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, "LEFT=back", TFT_CYAN, 1, TFT_BLACK);
                return;
            }

            char chip[64];
            snprintf(chip, sizeof(chip), "PN5%02lX fw %lu.%lu", (unsigned long)((version_ >> 24) & 0xFF), (unsigned long)((version_ >> 16) & 0xFF), (unsigned long)((version_ >> 8) & 0xFF));
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, chip, TFT_WHITE, 1, TFT_BLACK);

            char gpio[64];
            snprintf(gpio, sizeof(gpio), "GPIO P3: 0x%02X", (unsigned)p3_);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 58, gpio, TFT_CYAN, 1, TFT_BLACK);

            lines_.clear();
            items_.clear();

            lines_.push_back(std::string("P30 ") + (((p3_ >> PN532_GPIO_P30) & 1) ? "HIGH" : "LOW"));
            lines_.push_back(std::string("P31 ") + (((p3_ >> PN532_GPIO_P31) & 1) ? "HIGH" : "LOW"));
            lines_.push_back(std::string("P33 ") + (((p3_ >> PN532_GPIO_P33) & 1) ? "HIGH" : "LOW"));
            lines_.push_back(std::string("P35 ") + (((p3_ >> PN532_GPIO_P35) & 1) ? "HIGH" : "LOW"));

            for (auto &s : lines_)
                items_.push_back(s.c_str());

            const int16_t list_y = 78;
            int16_t y = list_y;
            for (size_t i = 0; i < items_.size(); i++)
            {
                const bool sel = i == selected_;
                const uint32_t fg = sel ? TFT_BLACK : TFT_WHITE;
                const uint32_t bg = sel ? TFT_CYAN : TFT_BLACK;
                Display::get_instance().fill_rect(flopper::ui::MARGIN_X - 4, y - 2, 240, flopper::ui::LINE_H, bg);
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, y, items_[i], fg, 1, bg);
                y += flopper::ui::LINE_H;
            }

            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 190, "UP/DN=select", TFT_CYAN, 1, TFT_BLACK);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 208, "CENTER=toggle  LEFT=back", TFT_CYAN, 1, TFT_BLACK);
        }

        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
            {
                exit();
                return;
            }
            if (!ok_)
                return;

            if (e == InputEvent::UP)
            {
                if (selected_ > 0)
                    selected_--;
                return;
            }
            if (e == InputEvent::DOWN)
            {
                if (selected_ + 1 < 4)
                    selected_++;
                return;
            }
            if (e == InputEvent::CENTER)
            {
                const uint8_t pin = selected_pin_bit_();
                p3_out_ ^= (1 << pin);
                if (!pn532::dev.writeGPIO(p3_out_))
                {
                    log::line("PN532", "writeGPIO failed");
                }
                p3_ = pn532::dev.readGPIO();
                return;
            }
        }

    private:
        bool ok_ = false;
        uint32_t version_ = 0;
        uint8_t p3_ = 0;
        uint8_t p3_out_ = 0;
        uint32_t last_poll_ms_ = 0;
        size_t selected_ = 0;

        std::vector<std::string> lines_;
        std::vector<const char *> items_;

        uint8_t selected_pin_bit_() const
        {
            switch (selected_)
            {
            case 0:
                return PN532_GPIO_P30;
            case 1:
                return PN532_GPIO_P31;
            case 2:
                return PN532_GPIO_P33;
            default:
                return PN532_GPIO_P35;
            }
        }
    };
}
