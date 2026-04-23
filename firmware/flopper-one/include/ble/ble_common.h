#pragma once

#include <NimBLEDevice.h>

namespace flopper::ble
{
    inline void ensure_init(const char *device_name = "flopper-one")
    {
        if (NimBLEDevice::isInitialized())
            return;

        NimBLEDevice::init(device_name);
        NimBLEDevice::setPower(ESP_PWR_LVL_P9);
    }

    inline bool set_static_random_addr(const char *addr)
    {
        if (!NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_RANDOM))
            return false;

        return NimBLEDevice::setOwnAddr(NimBLEAddress(addr, BLE_ADDR_RANDOM));
    }
}
