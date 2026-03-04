#pragma once
#include "../app.h"
#include "../display.h"
#include "../ui.h"

namespace flopper
{

    class HelloWorldApp : public App
    {
    public:
        HelloWorldApp() : App("Hello World") {}

        void on_enter() override {}
        void on_exit() override {}
        void tick() override { draw(); }
        void draw() override
        {
            flopper::ui::draw_status(Display::get_instance(), "Hello World!");
        }
        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
                exit();
        }
    };

}