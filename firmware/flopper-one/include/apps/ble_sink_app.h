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
            flopper::ble::ensure_init("flopper-one");
            entries_.clear();
            lines_.clear();
            selected_ = 0;
            status_ = "";
            state_ = State::Scanning;
            start_scan_();
            log::line("BLE", "sink enter");
        }

        void on_exit() override
        {
            stop_all_();
            log::line("BLE", "sink exit");
        }

        void tick() override
        {
            if (state_ == State::Scanning && millis() - last_restart_ms_ > 6000)
                start_scan_();
            draw();
        }

        void draw() override
        {
            const char *hdr = "BLE Sink";
            if (state_ == State::Scanning)
                hdr = "BLE Sink: pick device";
            else if (state_ == State::Connecting)
                hdr = "BLE Sink: connecting";
            else if (state_ == State::Connected)
                hdr = "BLE Sink: connected";
            else if (state_ == State::Error)
                hdr = "BLE Sink: error";

            flopper::ui::draw_status(Display::get_instance(), hdr);

            if (state_ == State::Scanning)
            {
                std::vector<const char *> items;
                lines_.clear();
                items.reserve(entries_.size());
                lines_.reserve(entries_.size());
                for (auto &e : entries_)
                {
                    char buf[96];
                    snprintf(buf, sizeof(buf), "%s %ddBm %s", e.addr.c_str(), e.rssi, e.name.size() ? e.name.c_str() : "");
                    lines_.push_back(buf);
                    items.push_back(lines_.back().c_str());
                }
                if (items.empty())
                    items.push_back("(no devices yet)");
                flopper::ui::draw_list(Display::get_instance(), items, flopper::ui::clamp_index(selected_, items.size()));
                return;
            }

            Display::get_instance().fill_rect(0, 30, 240, 210, TFT_BLACK);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, status_.c_str(), TFT_WHITE, 2, TFT_BLACK);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, "LEFT=back", TFT_CYAN, 2, TFT_BLACK);
        }

        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
            {
                exit();
                return;
            }

            if (state_ != State::Scanning)
                return;

            if (flopper::ui::apply_list_nav(e, selected_, entries_.size()))
                return;
            if (e == InputEvent::CENTER)
            {
                if (entries_.empty())
                    return;
                connect_(entries_[selected_]);
            }
        }

    private:
        enum class State
        {
            Scanning,
            Connecting,
            Connected,
            Error
        };

        struct Entry
        {
            std::string addr;
            uint8_t addr_type = 0;
            std::string name;
            int rssi = 0;
        };

        class ScanCallbacks : public NimBLEScanCallbacks
        {
        public:
            explicit ScanCallbacks(BleSinkApp &app) : app_(app) {}

            void onResult(const NimBLEAdvertisedDevice *d) override
            {
                if (!d)
                    return;

                Entry e;
                e.addr = d->getAddress().toString();
                e.addr_type = d->getAddressType();
                e.name = d->getName();
                e.rssi = d->getRSSI();
                app_.upsert_(e);
            }

        private:
            BleSinkApp &app_;
        };

        State state_ = State::Scanning;
        uint32_t last_restart_ms_ = 0;
        size_t selected_ = 0;

        std::vector<Entry> entries_;
        std::vector<std::string> lines_;
        std::string status_;

        NimBLEClient *client_ = nullptr;
        ScanCallbacks scan_cb_{*this};

        void upsert_(const Entry &e)
        {
            for (auto &x : entries_)
            {
                if (x.addr == e.addr)
                {
                    x = e;
                    return;
                }
            }
            entries_.push_back(e);
            if (entries_.size() > 30)
                entries_.erase(entries_.begin());
        }

        void start_scan_()
        {
            NimBLEScan *scan = NimBLEDevice::getScan();
            scan->setScanCallbacks(&scan_cb_, true);
            scan->setActiveScan(true);
            scan->setInterval(45);
            scan->setWindow(15);
            scan->start(5, false);
            last_restart_ms_ = millis();
        }

        void stop_scan_()
        {
            NimBLEDevice::getScan()->stop();
        }

        void connect_(const Entry &target)
        {
            stop_scan_();
            state_ = State::Connecting;
            status_ = std::string("connecting ") + target.addr;
            log::printf("BLE", "sink connect %s %s", target.addr.c_str(), target.name.c_str());

            client_ = NimBLEDevice::createClient();
            if (!client_->connect(NimBLEAddress(target.addr, target.addr_type)))
            {
                state_ = State::Error;
                status_ = "connect failed";
                return;
            }

            state_ = State::Connected;
            status_ = std::string("connected ") + target.addr;
        }

        void stop_all_()
        {
            stop_scan_();
            if (client_)
            {
                if (client_->isConnected())
                    client_->disconnect();
                NimBLEDevice::deleteClient(client_);
                client_ = nullptr;
            }
            state_ = State::Scanning;
        }
    };
}
