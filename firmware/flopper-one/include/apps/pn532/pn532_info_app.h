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
            ok_ = false;
            first_frame_ = true;
            version_ = 0;
            p3_ = 0;
            p3_out_ = 0;
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
            if (first_frame_)
            {
                first_frame_ = false;
                draw();
                return;
            }

            if (!ok_)
            {
                ok_ = (pn532::tick_init() == pn532::InitState::Ready);
                if (ok_)
                {
                    version_ = pn532::firmware_version;
                    p3_ = pn532::dev.readGPIO();
                    p3_out_ = p3_;
                    last_poll_ms_ = millis();
                }
                draw();
                return;
            }

            if (millis() - last_poll_ms_ > 500)
            {
                p3_ = pn532::dev.readGPIO();
                last_poll_ms_ = millis();
            }
            draw();
        }

        void draw() override
        {
            char title[64];
            if (ok_)
                snprintf(title, sizeof(title), "PN532 Info");
            else
                snprintf(title, sizeof(title), "PN532 Info (%s)", pn532::init_state_label(pn532::init_state));
            flopper::ui::draw_status(Display::get_instance(), title);
            Display::get_instance().fill_rect(0, 30, 240, 210, flopper::ui::BACKGROUND_COLOR);

            if (!ok_)
            {
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40,
                                                  pn532::init_state == pn532::InitState::InProgress || pn532::init_state == pn532::InitState::NotStarted
                                                      ? "Initializing..."
                                                      : "No PN532 detected",
                                                  flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, "LEFT=back", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
                return;
            }

            char chip[64];
            snprintf(chip, sizeof(chip), "PN5%02lX fw %lu.%lu", (unsigned long)((version_ >> 24) & 0xFF), (unsigned long)((version_ >> 16) & 0xFF), (unsigned long)((version_ >> 8) & 0xFF));
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, chip, flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);

            char gpio[64];
            snprintf(gpio, sizeof(gpio), "GPIO P3: 0x%02X", (unsigned)p3_);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 58, gpio, flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);

            lines_.clear();
            items_.clear();

            lines_.push_back(std::string("P30 ") + (((p3_ >> PN532_GPIO_P30) & 1) ? "HIGH" : "LOW"));
            lines_.push_back(std::string("P31 ") + (((p3_ >> PN532_GPIO_P31) & 1) ? "HIGH" : "LOW"));
            lines_.push_back(std::string("P33 ") + (((p3_ >> PN532_GPIO_P33) & 1) ? "HIGH" : "LOW"));
            lines_.push_back(std::string("P35 ") + (((p3_ >> PN532_GPIO_P35) & 1) ? "HIGH" : "LOW"));

            for (auto &s : lines_)
                items_.push_back(s.c_str());

            flopper::ui::draw_list_at(Display::get_instance(), items_, flopper::ui::clamp_index(selected_, items_.size()), 78);

            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 190, "UP/DN=select", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 208, "CENTER=toggle  LEFT=back", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
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

            if (flopper::ui::apply_list_nav(e, selected_, 4))
                return;
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
        bool first_frame_ = true;
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
