#pragma once

#include "../app.h"
#include "../display.h"
#include "../ui.h"
#include "../log.h"
#include "../ir_shared.h"
#include "../flopper_pins.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>

namespace flopper
{
    class IrEmitterApp : public App
    {
    public:
        IrEmitterApp() : App("IR Emit") {}

        void on_enter() override
        {
            if (!started_)
            {
                started_ = true;
                irsend_.begin();
            }
            log::line("IR", "emitter enter");
        }

        void on_exit() override
        {
            log::line("IR", "emitter exit");
        }

        void tick() override
        {
            draw();
        }

        void draw() override
        {
            flopper::ui::draw_status(Display::get_instance(), "IR Emit (CTR=send)");
            std::vector<const char *> items;
            items.push_back("NEC demo (0x20DF10EF)");
            items.push_back("Sony demo (0xA90,12)");
            items.push_back("RC5 demo (0x300C,12)");
            items.push_back(ir_shared::last_raw_len ? "Replay last raw" : "Replay last raw (none)");
            flopper::ui::draw_list(Display::get_instance(), items, selected_);
        }

        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
            {
                exit();
                return;
            }
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
            if (e != InputEvent::CENTER)
                return;

            if (selected_ == 0)
            {
                irsend_.sendNEC(0x20DF10EF, 32);
                log::line("IR", "send NEC 0x20DF10EF");
            }
            else if (selected_ == 1)
            {
                irsend_.sendSony(0xA90, 12);
                log::line("IR", "send SONY 0xA90/12");
            }
            else if (selected_ == 2)
            {
                irsend_.sendRC5(0x300C, 12);
                log::line("IR", "send RC5 0x300C/12");
            }
            else if (selected_ == 3)
            {
                if (ir_shared::last_raw_len)
                {
                    irsend_.sendRaw(ir_shared::last_raw_us, ir_shared::last_raw_len, ir_shared::last_raw_khz);
                    log::printf("IR", "replay raw len=%u khz=%u", (unsigned)ir_shared::last_raw_len, (unsigned)ir_shared::last_raw_khz);
                }
            }
        }

    private:
        bool started_ = false;
        size_t selected_ = 0;
        IRsend irsend_{pins::IR_LED};
    };
}
