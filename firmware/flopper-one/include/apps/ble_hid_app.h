#pragma once

#include "../app.h"
#include "../display.h"
#include "../ui.h"
#include "../log.h"
#include "../ble/ble_common.h"

#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

namespace flopper
{
    class BleHidApp : public App
    {
    public:
        BleHidApp() : App("BLE HID") {}

        void on_enter() override
        {
            flopper::ble::ensure_init("flopper-one");
            if (!started_)
                start_();
            log::line("BLE", "hid enter");
        }

        void on_exit() override
        {
            log::line("BLE", "hid exit");
        }

        void tick() override { draw(); }

        void draw() override
        {
            flopper::ui::draw_status(Display::get_instance(), "BLE HID (CTR=type, LEFT=back)");
            Display::get_instance().fill_rect(0, 30, 240, 210, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, connected_ ? "status: connected" : "status: not connected", connected_ ? flopper::ui::SUCCESS_COLOR : flopper::ui::FAILURE_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, "Pairs as a keyboard", flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
        }

        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
            {
                exit();
                return;
            }
            if (e == InputEvent::CENTER)
            {
                if (!connected_ || !input_)
                    return;
                send_string_("hello world\n");
                log::line("BLE", "hid typed hello world");
            }
        }

    private:
        class ServerCallbacks : public NimBLEServerCallbacks
        {
        public:
            explicit ServerCallbacks(BleHidApp &app) : app_(app) {}

            void onConnect(NimBLEServer *, NimBLEConnInfo &) override { app_.connected_ = true; }
            void onDisconnect(NimBLEServer *, NimBLEConnInfo &, int) override
            {
                app_.connected_ = false;
                NimBLEDevice::startAdvertising();
            }

        private:
            BleHidApp &app_;
        };

        bool started_ = false;
        bool connected_ = false;

        NimBLEServer *server_ = nullptr;
        NimBLEHIDDevice *hid_ = nullptr;
        NimBLECharacteristic *input_ = nullptr;
        ServerCallbacks server_cb_{*this};

        void start_()
        {
            started_ = true;

            server_ = NimBLEDevice::createServer();
            server_->setCallbacks(&server_cb_);

            hid_ = new NimBLEHIDDevice(server_);
            input_ = hid_->getInputReport(1);
            hid_->getOutputReport(1);

            static const uint8_t report_map[] = {
                0x05, 0x01,       // Usage Page (Generic Desktop)
                0x09, 0x06,       // Usage (Keyboard)
                0xA1, 0x01,       // Collection (Application)
                0x05, 0x07,       //   Usage Page (Key Codes)
                0x19, 0xE0,       //   Usage Minimum (224)
                0x29, 0xE7,       //   Usage Maximum (231)
                0x15, 0x00,       //   Logical Minimum (0)
                0x25, 0x01,       //   Logical Maximum (1)
                0x75, 0x01,       //   Report Size (1)
                0x95, 0x08,       //   Report Count (8)
                0x81, 0x02,       //   Input (Data, Variable, Absolute) ; Modifier byte
                0x95, 0x01,       //   Report Count (1)
                0x75, 0x08,       //   Report Size (8)
                0x81, 0x01,       //   Input (Constant) ; Reserved byte
                0x95, 0x05,       //   Report Count (5)
                0x75, 0x01,       //   Report Size (1)
                0x05, 0x08,       //   Usage Page (LEDs)
                0x19, 0x01,       //   Usage Minimum (1)
                0x29, 0x05,       //   Usage Maximum (5)
                0x91, 0x02,       //   Output (Data, Variable, Absolute) ; LED report
                0x95, 0x01,       //   Report Count (1)
                0x75, 0x03,       //   Report Size (3)
                0x91, 0x01,       //   Output (Constant) ; LED report padding
                0x95, 0x06,       //   Report Count (6)
                0x75, 0x08,       //   Report Size (8)
                0x15, 0x00,       //   Logical Minimum (0)
                0x25, 0x65,       //   Logical Maximum (101)
                0x05, 0x07,       //   Usage Page (Key Codes)
                0x19, 0x00,       //   Usage Minimum (0)
                0x29, 0x65,       //   Usage Maximum (101)
                0x81, 0x00,       //   Input (Data, Array)
                0xC0              // End Collection
            };

            hid_->setReportMap((uint8_t *)report_map, sizeof(report_map));
            hid_->setBatteryLevel(100);
            hid_->startServices();

            NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
            adv->setAppearance(HID_KEYBOARD);
            adv->addServiceUUID(hid_->getHidService()->getUUID());
            adv->start();
        }

        struct HidKey
        {
            uint8_t mod;
            uint8_t key;
        };

        static HidKey map_key_(char c)
        {
            // Modifier bits: 0x02 = Left Shift
            if (c >= 'a' && c <= 'z')
                return {0x00, (uint8_t)(0x04 + (c - 'a'))};
            if (c >= 'A' && c <= 'Z')
                return {0x02, (uint8_t)(0x04 + (c - 'A'))};
            if (c >= '1' && c <= '9')
                return {0x00, (uint8_t)(0x1E + (c - '1'))};
            if (c == '0')
                return {0x00, 0x27};
            if (c == ' ')
                return {0x00, 0x2C};
            if (c == '\n')
                return {0x00, 0x28};
            if (c == '.')
                return {0x00, 0x37};
            if (c == ',')
                return {0x00, 0x36};
            if (c == '-')
                return {0x00, 0x2D};
            if (c == '_')
                return {0x02, 0x2D};
            if (c == '=')
                return {0x00, 0x2E};
            if (c == '+')
                return {0x02, 0x2E};
            if (c == '/')
                return {0x00, 0x38};
            if (c == ':')
                return {0x02, 0x33};
            if (c == ';')
                return {0x00, 0x33};
            if (c == '!')
                return {0x02, 0x1E};
            return {0x00, 0x00};
        }

        void send_key_(HidKey k)
        {
            uint8_t report[8] = {0};
            report[0] = k.mod;
            report[2] = k.key;
            input_->setValue(report, sizeof(report));
            input_->notify();
            delay(8);
            memset(report, 0, sizeof(report));
            input_->setValue(report, sizeof(report));
            input_->notify();
            delay(8);
        }

        void send_string_(const char *s)
        {
            for (const char *p = s; *p; p++)
            {
                const HidKey k = map_key_(*p);
                if (k.key)
                    send_key_(k);
            }
        }
    };
}
