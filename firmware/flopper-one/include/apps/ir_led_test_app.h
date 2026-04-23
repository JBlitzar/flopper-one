#pragma once

#include "../app.h"
#include "../display.h"
#include "../ui.h"
#include "../log.h"
#include "../flopper_pins.h"

namespace flopper
{
    class IrLedTestApp : public App
    {
    public:
        IrLedTestApp() : App("IR LED Test") {}

        void on_enter() override
        {
            pinMode(pins::IR_LED, OUTPUT);
            digitalWrite(pins::IR_LED, HIGH);
            active_ = true;
            log::line("IR", "led test enter");
        }

        void on_exit() override
        {
            active_ = false;
            digitalWrite(pins::IR_LED, LOW);
            log::line("IR", "led test exit");
        }

        void tick() override { draw(); }

        void draw() override
        {
            flopper::ui::draw_status(Display::get_instance(), "IR LED Test: on");
            Display::get_instance().fill_rect(0, 30, 240, 210, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, "IR LED is forced on", flopper::ui::SUCCESS_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 70, "LEFT=back", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
        }

        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
            {
                exit();
            }
        }

    private:
        bool active_ = false;
    };
}