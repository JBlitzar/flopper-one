#pragma once

#include "../../app.h"
#include "../../display.h"
#include "../../ui.h"
#include "../../log.h"

#include "pn532_common.h"

#include <vector>

namespace flopper
{
    class Pn532TargetApp : public App
    {
    public:
        Pn532TargetApp() : App("Target (echo)") {}

        void on_enter() override
        {
            ok_ = pn532::ensure_init();
            running_ = false;
            in_target_ = false;
            rx_len_ = 0;
            last_rx_.clear();
            last_tx_.clear();
            last_poll_ms_ = 0;
            log::line("PN532", "target enter");
        }

        void on_exit() override
        {
            running_ = false;
            in_target_ = false;
            log::line("PN532", "target exit");
        }

        void tick() override
        {
            if (ok_ && running_ && millis() - last_poll_ms_ > 250)
            {
                step_();
                last_poll_ms_ = millis();
            }
            draw();
        }

        void draw() override
        {
            if (!ok_)
            {
                flopper::ui::draw_status(Display::get_instance(), "PN532 Target (not found)");
                Display::get_instance().fill_rect(0, 30, 240, 210, flopper::ui::BACKGROUND_COLOR);
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, "No PN532 detected", flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, "LEFT=back", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
                return;
            }

            flopper::ui::draw_status(Display::get_instance(), running_ ? "PN532 Target (running)" : "PN532 Target (stopped)");
            Display::get_instance().fill_rect(0, 30, 240, 210, flopper::ui::BACKGROUND_COLOR);

            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, in_target_ ? "mode: target" : "mode: idle", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, "Last RX:", flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 78, last_rx_.size() ? last_rx_.c_str() : "(none)", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 110, "Last TX:", flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 128, last_tx_.size() ? last_tx_.c_str() : "(none)", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);

            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 170, "CENTER=start/stop", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 188, "LEFT=back", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
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
            if (e == InputEvent::CENTER)
            {
                running_ = !running_;
                in_target_ = false;
                last_rx_.clear();
                last_tx_.clear();
                return;
            }
        }

    private:
        bool ok_ = false;
        bool running_ = false;
        bool in_target_ = false;
        uint32_t last_poll_ms_ = 0;

        uint8_t rx_[64] = {0};
        uint8_t rx_len_ = 0;
        std::string last_rx_;
        std::string last_tx_;

        void step_()
        {
            if (!in_target_)
            {
                const uint8_t ok = pn532::dev.AsTarget();
                in_target_ = ok;
                if (!ok)
                    log::line("PN532", "AsTarget failed");
                else
                    log::line("PN532", "AsTarget ok");
                return;
            }

            rx_len_ = 0;
            const uint8_t ok = pn532::dev.getDataTarget(rx_, &rx_len_);
            if (!ok || rx_len_ == 0)
                return;

            last_rx_ = pn532::bytes_to_hex(rx_, rx_len_);
            flopper::log::printf("PN532", "rx %u bytes: %s", (unsigned)rx_len_, last_rx_.c_str());

            uint8_t tx[66] = {0};
            const uint8_t echo_len = rx_len_ > 60 ? 60 : rx_len_;
            tx[0] = 0x8E;
            memcpy(tx + 1, rx_, echo_len);
            const uint8_t tok = pn532::dev.setDataTarget(tx, echo_len + 1);
            last_tx_ = tok ? pn532::bytes_to_hex(rx_, echo_len) : std::string("send failed");
        }
    };
}
