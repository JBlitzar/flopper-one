#pragma once
#include "../app.h"
#include "../display.h"
#include "../ui.h"

namespace flopper
{

    class HelloWorldAppTwo : public App
    {
    public:
        HelloWorldAppTwo() : App("Hello World Two") {}

        void on_enter() override {}
        void on_exit() override {}
        void tick() override { draw(); }
        void draw() override
        {
            flopper::ui::draw_status(Display::get_instance(), "Hello World! 2");
        }
        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
                exit();
        }
    };

}