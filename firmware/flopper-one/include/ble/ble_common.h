#pragma once

#include <NimBLEDevice.h>

namespace flopper::ble
{
    inline void ensure_init(const char *device_name = "flopper-one")
    {
        static bool inited = false;
        if (inited)
            return;
        inited = true;

        NimBLEDevice::init(device_name);
        NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    }
}
