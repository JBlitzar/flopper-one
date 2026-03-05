#pragma once

#include "../app.h"
#include "../display.h"
#include "../ui.h"
#include "../log.h"
#include "../ir_shared.h"
#include "../flopper_pins.h"

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

namespace flopper
{
    class IrSnifferApp : public App
    {
    public:
        IrSnifferApp() : App("IR Sniff") {}

        void on_enter() override
        {
            if (!started_)
            {
                started_ = true;
                irrecv_.setUnknownThreshold(12);
                irrecv_.enableIRIn();
                log::line("IR", "sniffer start");
            }
        }

        void on_exit() override
        {
            log::line("IR", "sniffer exit");
        }

        void tick() override
        {
            decode_results results;
            if (irrecv_.decode(&results))
            {
                const String basic = resultToHumanReadableBasic(&results);
                ir_shared::last_summary = basic;

                // Save raw timings for replay.
                ir_shared::last_raw_len = 0;
                // results.rawbuf[0] is gap; start at 1.
                for (uint16_t i = 1; i < results.rawlen && ir_shared::last_raw_len < (sizeof(ir_shared::last_raw_us) / sizeof(ir_shared::last_raw_us[0])); i++)
                {
                    ir_shared::last_raw_us[ir_shared::last_raw_len++] = results.rawbuf[i] * kRawTick;
                }

                log::line("IR", basic.c_str());
                irrecv_.resume();
            }

            draw();
        }

        void draw() override
        {
            flopper::ui::draw_status(Display::get_instance(), "IR Sniff (LEFT=back)");
            Display::get_instance().fill_rect(0, 30, 240, 210, TFT_BLACK);

            const char *s = ir_shared::last_summary.length() ? ir_shared::last_summary.c_str() : "(waiting...)";
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, s, TFT_WHITE, 1, TFT_BLACK);
            char buf[64];
            snprintf(buf, sizeof(buf), "raw=%u", (unsigned)ir_shared::last_raw_len);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, buf, TFT_CYAN, 1, TFT_BLACK);
        }

        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
                exit();
        }

    private:
        bool started_ = false;
        IRrecv irrecv_{pins::TSOP_OUT, 1024, 15, true};
    };
}
