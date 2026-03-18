#pragma once

#include "../app.h"
#include "../display.h"
#include "../ui.h"
#include "../log.h"
#include "../ir_shared.h"
#include "../flopper_pins.h"

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <vector>

namespace flopper
{
    struct IRCommand
    {
        const char *name;
        decode_type_t protocol;
        uint64_t code;
        uint16_t bits;
    };
    // https://github.com/OMOTE-Community/OMOTE-Firmware/blob/main/src/devices/mediaPlayer/device_appleTV/device_appleTV.cpp
    static const IRCommand SAMSUNG_COMMANDS[] = {
        {"Power On", SAMSUNG, 0xE0E09966, 32},
        {"Power Off", SAMSUNG, 0xE0E019E6, 32},
        {"HDMI 1", SAMSUNG, 0xE0E09768, 32},
        {"HDMI 2", SAMSUNG, 0xE0E07D82, 32},
        {"HDMI 3", SAMSUNG, 0xE0E043BC, 32},
        {"Vol Up", SAMSUNG, 0xE0E0E01F, 32},
        {"Vol Down", SAMSUNG, 0xE0E0D02F, 32},
        {"Mute", SAMSUNG, 0xE0E0F00F, 32},
        {"Ch Up", SAMSUNG, 0xE0E048B7, 32},
        {"Ch Down", SAMSUNG, 0xE0E008F7, 32},
    };

    static const IRCommand APPLETV_COMMANDS[] = {
        {"Up", NEC, 0x77E15080, 32},
        {"Down", NEC, 0x77E13080, 32},
        {"Left", NEC, 0x77E19080, 32},
        {"Right", NEC, 0x77E16080, 32},
        {"OK", NEC, 0x77E13A80, 32},
        {"Menu", NEC, 0x77E1C080, 32},
        {"Play", NEC, 0x77E1FA80, 32},
        {"Pause", NEC, 0xA7E14C80, 32},
        {"Power On", NEC, 0xA7E10280, 32},
        {"Power Off", NEC, 0xA7E10280, 32},
    };
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
            if (!device_selected_)
            {
                static const char *devices[] = {"Samsung TV", "Apple TV"};
                std::vector<const char *> items(devices, devices + 2);
                flopper::ui::draw_list(Display::get_instance(), items, flopper::ui::clamp_index(selected_, items.size()));
                return;
            }

            const IRCommand *cmds = current_device_ == 0 ? SAMSUNG_COMMANDS : APPLETV_COMMANDS;
            size_t count = current_device_ == 0
                               ? sizeof(SAMSUNG_COMMANDS) / sizeof(IRCommand)
                               : sizeof(APPLETV_COMMANDS) / sizeof(IRCommand);

            std::vector<const char *> items;
            for (size_t i = 0; i < count; i++)
                items.push_back(cmds[i].name);
            items.push_back(ir_shared::last_raw_len ? "Replay last raw" : "Replay last raw (none)");
            flopper::ui::draw_list(Display::get_instance(), items, flopper::ui::clamp_index(selected_, items.size()));
        }

        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
            {
                if (device_selected_)
                {
                    device_selected_ = false;
                    selected_ = 0;
                    return;
                }
                exit();
                return;
            }

            const IRCommand *cmds = current_device_ == 0 ? SAMSUNG_COMMANDS : APPLETV_COMMANDS;
            size_t count = current_device_ == 0
                               ? sizeof(SAMSUNG_COMMANDS) / sizeof(IRCommand)
                               : sizeof(APPLETV_COMMANDS) / sizeof(IRCommand);
            size_t total = device_selected_ ? count + 1 : 2; // +1 for raw replay

            if (flopper::ui::apply_list_nav(e, selected_, total))
                return;
            if (e != InputEvent::CENTER)
                return;

            if (!device_selected_)
            {
                current_device_ = selected_;
                device_selected_ = true;
                selected_ = 0;
                return;
            }

            // send command
            if (selected_ == count)
            {
                // raw replay
                if (ir_shared::last_raw_len)
                {
                    irsend_.sendRaw(ir_shared::last_raw_us, ir_shared::last_raw_len, ir_shared::last_raw_khz);
                    log::printf("IR", "replay raw len=%u khz=%u", (unsigned)ir_shared::last_raw_len, (unsigned)ir_shared::last_raw_khz);
                }
                return;
            }

            const auto &cmd = cmds[selected_];
            irsend_.send(cmd.protocol, cmd.code, cmd.bits);
            log::printf("IR", "sent %s", cmd.name);
        }

    private:
        bool started_ = false;
        bool device_selected_ = false;
        size_t current_device_ = 0;
        size_t selected_ = 0;
        IRsend irsend_{pins::IR_LED};
    };
}
