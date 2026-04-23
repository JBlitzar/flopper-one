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
            flopper::ble::ensure_init("flopper-hid");
            NimBLEDevice::setDeviceName("Flopper HID");
            flopper::ble::set_static_random_addr("D3:16:12:34:56:71");
            NimBLEDevice::setSecurityAuth(true, false, true);
            NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
            NimBLEDevice::setSecurityInitKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);
            NimBLEDevice::setSecurityRespKey(BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID);

            active_ = true;
            if (!started_)
                start_();
            start_advertising_();
            log::line("BLE", "hid enter");
        }

        void on_exit() override
        {
            active_ = false;
            stop_advertising_();
            if (server_ && connected_ && conn_handle_ != kInvalidConnHandle)
                server_->disconnect(conn_handle_);

            if (NimBLEDevice::isInitialized())
                NimBLEDevice::deinit(true);

            started_ = false;
            server_ = nullptr;
            hid_ = nullptr;
            input_ = nullptr;
            advertising_ = nullptr;
            connected_ = false;
            conn_handle_ = kInvalidConnHandle;
            log::line("BLE", "hid exit");
        }

        void tick() override { draw(); }

        void draw() override
        {
            const char *hdr = "BLE HID: advertising";
            if (secure_)
                hdr = "BLE HID: connected";
            else if (connected_)
                hdr = "BLE HID: pairing";

            flopper::ui::draw_status(Display::get_instance(), hdr);
            Display::get_instance().fill_rect(0, 30, 240, 210, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, "Pairs as a keyboard", flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, secure_ ? "CENTER=type hello world" : (connected_ ? "pairing with Apple device" : "waiting for a central"), secure_ ? flopper::ui::SUCCESS_COLOR : flopper::ui::MUTED_TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 80, status_.c_str(), flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 110, "LEFT=back", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
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
                if (!connected_ || !secure_ || !input_)
                    return;
                send_string_("hello world\n");
                log::line("BLE", "hid typed hello world");
            }
        }

    private:
        static constexpr uint16_t kInvalidConnHandle = 0xFFFF;

        class ServerCallbacks : public NimBLEServerCallbacks
        {
        public:
            explicit ServerCallbacks(BleHidApp &app) : app_(app) {}

            void onConnect(NimBLEServer *server, NimBLEConnInfo &info) override
            {
                app_.connected_ = true;
                app_.secure_ = false;
                app_.conn_handle_ = info.getConnHandle();
                app_.status_ = "pairing";
                if (server)
                    server->updateConnParams(info.getConnHandle(), 12, 24, 0, 200);
                NimBLEDevice::startSecurity(info.getConnHandle());
                log::line("BLE", "hid connected");
            }

            void onAuthenticationComplete(NimBLEConnInfo &connInfo) override
            {
                if (!connInfo.isEncrypted())
                {
                    app_.status_ = "auth failed";
                    app_.secure_ = false;
                    if (app_.server_)
                        app_.server_->disconnect(connInfo.getConnHandle());
                    return;
                }

                app_.secure_ = true;
                app_.status_ = connInfo.isBonded() ? "secure bonded" : "secure";
                log::line("BLE", "hid authenticated");
            }

            void onDisconnect(NimBLEServer *, NimBLEConnInfo &, int reason) override
            {
                app_.connected_ = false;
                app_.secure_ = false;
                app_.conn_handle_ = kInvalidConnHandle;
                app_.status_ = "disconnected";
                if (app_.active_)
                    NimBLEDevice::startAdvertising();
                log::printf("BLE", "hid disconnected reason=%d", reason);
            }

        private:
            BleHidApp &app_;
        };

        bool started_ = false;
        bool active_ = false;
        bool connected_ = false;
        bool secure_ = false;
        uint16_t conn_handle_ = kInvalidConnHandle;

        NimBLEServer *server_ = nullptr;
        NimBLEHIDDevice *hid_ = nullptr;
        NimBLECharacteristic *input_ = nullptr;
        NimBLEAdvertising *advertising_ = nullptr;
        std::string status_ = "advertising";
        ServerCallbacks server_cb_{*this};

        void start_()
        {
            server_ = NimBLEDevice::createServer();
            server_->setCallbacks(&server_cb_, false);
            server_->advertiseOnDisconnect(false);

            hid_ = new NimBLEHIDDevice(server_);
            input_ = hid_->getInputReport(1);
            hid_->getOutputReport(1);

            static const uint8_t report_map[] = {
                0x05, 0x01, // Usage Page (Generic Desktop)
                0x85, 0x01, // Report ID 1
                0x09, 0x06, // Usage (Keyboard)
                0xA1, 0x01, // Collection (Application)
                0x05, 0x07, //   Usage Page (Key Codes)
                0x19, 0xE0, //   Usage Minimum (224)
                0x29, 0xE7, //   Usage Maximum (231)
                0x15, 0x00, //   Logical Minimum (0)
                0x25, 0x01, //   Logical Maximum (1)
                0x75, 0x01, //   Report Size (1)
                0x95, 0x08, //   Report Count (8)
                0x81, 0x02, //   Input (Data, Variable, Absolute) ; Modifier byte
                0x95, 0x01, //   Report Count (1)
                0x75, 0x08, //   Report Size (8)
                0x81, 0x01, //   Input (Constant) ; Reserved byte
                0x95, 0x05, //   Report Count (5)
                0x75, 0x01, //   Report Size (1)
                0x05, 0x08, //   Usage Page (LEDs)
                0x19, 0x01, //   Usage Minimum (1)
                0x29, 0x05, //   Usage Maximum (5)
                0x91, 0x02, //   Output (Data, Variable, Absolute) ; LED report
                0x95, 0x01, //   Report Count (1)
                0x75, 0x03, //   Report Size (3)
                0x91, 0x01, //   Output (Constant) ; LED report padding
                0x95, 0x06, //   Report Count (6)
                0x75, 0x08, //   Report Size (8)
                0x15, 0x00, //   Logical Minimum (0)
                0x25, 0x65, //   Logical Maximum (101)
                0x05, 0x07, //   Usage Page (Key Codes)
                0x19, 0x00, //   Usage Minimum (0)
                0x29, 0x65, //   Usage Maximum (101)
                0x81, 0x00, //   Input (Data, Array)
                0xC0        // End Collection
            };

            hid_->setReportMap((uint8_t *)report_map, sizeof(report_map));
            hid_->setManufacturer("Flopper");
            hid_->setPnp(0x02, 0xF10F, 0x0001, 0x0001);
            hid_->setHidInfo(0, 0x00);
            hid_->getBootInput();
            hid_->getBootOutput();
            hid_->setBatteryLevel(100);
            hid_->startServices();

            advertising_ = NimBLEDevice::getAdvertising();
            advertising_->setName("Flopper HID");
            advertising_->setAppearance(HID_KEYBOARD);
            advertising_->addServiceUUID(hid_->getHidService()->getUUID());
            advertising_->enableScanResponse(true);

            started_ = true;
        }

        void start_advertising_()
        {
            if (!advertising_)
                return;

            if (!advertising_->isAdvertising())
                advertising_->start();
            status_ = "advertising";
        }

        void stop_advertising_()
        {
            if (advertising_ && advertising_->isAdvertising())
                advertising_->stop();
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
