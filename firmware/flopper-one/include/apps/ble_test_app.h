#pragma once
#include "../app.h"
#include "../display.h"
#include "../ui.h"
#include "../ble/ble_test.h"

namespace flopper
{

    class BleTestApp : public App
    {
    public:
        BleTestApp() : App("BLE test app.") {}

        void on_enter() override
        {
            _app_main();
        }
        void on_exit() override {}
        void tick() override { draw(); }
        void draw() override
        {
        }
        void on_input(InputEvent e) override
        {
            // power cycle to exit haha
        }
    };

}