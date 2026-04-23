#pragma once

#include "../app.h"
#include "../display.h"
#include "../ui.h"
#include "../log.h"
#include "../ble/ble_common.h"

#include <NimBLEDevice.h>

namespace flopper
{
    class BleSinkApp : public App
    {
    public:
        BleSinkApp() : App("BLE Sink") {}

        void on_enter() override
        {
            flopper::ble::ensure_init("flopper-sink");
            NimBLEDevice::setDeviceName("Flopper Sink");
            flopper::ble::set_static_random_addr("D3:16:12:34:56:70");

            active_ = true;
            if (!started_)
                start_();

            start_advertising_();
            log::line("BLE", "sink enter");
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
            service_ = nullptr;
            characteristic_ = nullptr;
            advertising_ = nullptr;
            connected_ = false;
            conn_handle_ = kInvalidConnHandle;
            log::line("BLE", "sink exit");
        }

        void tick() override { draw(); }

        void draw() override
        {
            flopper::ui::draw_status(Display::get_instance(), connected_ ? "BLE Sink: connected" : "BLE Sink: advertising");

            Display::get_instance().fill_rect(0, 30, 240, 210, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, "Peripheral sink mode", flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, connected_ ? "CENTER=notify ping" : "Waiting for a central", connected_ ? flopper::ui::SUCCESS_COLOR : flopper::ui::MUTED_TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 80, status_.c_str(), flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 110, last_message_.c_str(), flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 140, "LEFT=back", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
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
                if (characteristic_)
                {
                    last_message_ = connected_ ? "sink ping" : "not connected";
                    characteristic_->setValue(last_message_.c_str());
                    if (connected_)
                        characteristic_->notify();
                    status_ = connected_ ? "sent notification" : "not connected";
                }
                return;
            }
        }

    private:
        static constexpr uint16_t kInvalidConnHandle = 0xFFFF;
        static constexpr const char *kServiceUuid = "F1A0";
        static constexpr const char *kCharacteristicUuid = "F1A1";

        class ServerCallbacks : public NimBLEServerCallbacks
        {
        public:
            explicit ServerCallbacks(BleSinkApp &app) : app_(app) {}

            void onConnect(NimBLEServer *server, NimBLEConnInfo &info) override
            {
                app_.connected_ = true;
                app_.conn_handle_ = info.getConnHandle();
                app_.status_ = "connected";
                if (server)
                    server->updateConnParams(info.getConnHandle(), 12, 24, 0, 200);
                log::line("BLE", "sink connected");
            }

            void onDisconnect(NimBLEServer *, NimBLEConnInfo &, int reason) override
            {
                app_.connected_ = false;
                app_.conn_handle_ = kInvalidConnHandle;
                app_.status_ = "disconnected";
                if (app_.active_)
                    NimBLEDevice::startAdvertising();
                log::printf("BLE", "sink disconnected reason=%d", reason);
            }

        private:
            BleSinkApp &app_;
        };

        class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
        {
        public:
            explicit CharacteristicCallbacks(BleSinkApp &app) : app_(app) {}

            void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &) override
            {
                if (!c)
                    return;

                app_.last_message_ = c->getValue();
                app_.status_ = "write received";
                log::printf("BLE", "sink got %u bytes", (unsigned)app_.last_message_.size());
            }

        private:
            BleSinkApp &app_;
        };

        bool active_ = false;
        bool started_ = false;
        bool connected_ = false;
        uint16_t conn_handle_ = kInvalidConnHandle;

        NimBLEServer *server_ = nullptr;
        NimBLEService *service_ = nullptr;
        NimBLECharacteristic *characteristic_ = nullptr;
        NimBLEAdvertising *advertising_ = nullptr;

        std::string status_ = "advertising";
        std::string last_message_ = "sink ready";

        ServerCallbacks server_cb_{*this};
        CharacteristicCallbacks char_cb_{*this};

        void start_()
        {
            server_ = NimBLEDevice::createServer();
            server_->setCallbacks(&server_cb_, false);
            server_->advertiseOnDisconnect(false);

            service_ = server_->createService(kServiceUuid);
            characteristic_ = service_->createCharacteristic(
                kCharacteristicUuid,
                NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::NOTIFY);
            characteristic_->setCallbacks(&char_cb_);
            characteristic_->setValue(last_message_.c_str());
            service_->start();

            advertising_ = NimBLEDevice::getAdvertising();
            advertising_->setName("Flopper Sink");
            advertising_->addServiceUUID(kServiceUuid);
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

        void update_last_message_(const std::string &msg)
        {
            last_message_ = msg;
            if (characteristic_)
            {
                characteristic_->setValue(last_message_.c_str());
                if (connected_)
                    characteristic_->notify();
            }
        }
    };
}
