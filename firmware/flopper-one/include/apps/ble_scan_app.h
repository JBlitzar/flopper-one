#pragma once

#include "../app.h"
#include "../display.h"
#include "../ui.h"
#include "../log.h"
#include "../ble/ble_common.h"

#include <NimBLEDevice.h>

namespace flopper
{
    struct BleScanEntry
    {
        std::string addr;
        std::string name;
        int rssi = 0;
    };

    class BleScanApp : public App
    {
    public:
        BleScanApp() : App("BLE Scan") {}

        void on_enter() override
        {
            flopper::ble::ensure_init();
            start_scan_();
            log::line("BLE", "scan enter");
        }

        void on_exit() override
        {
            stop_scan_();
            log::line("BLE", "scan exit");
        }

        void tick() override
        {
            if (scanning_ && millis() - last_restart_ms_ > 6000)
            {
                // keep scan running in chunks
                start_scan_();
            }
            draw();
        }

        void draw() override
        {
            flopper::ui::draw_status(Display::get_instance(), scanning_ ? "BLE Scan (CTR=toggle)" : "BLE Scan (stopped)");
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
                scanning_ ? stop_scan_() : start_scan_();
                return;
            }

            if (flopper::ui::apply_list_nav(e, selected_, entries_.size()))
                return;
        }

    private:
        class Callbacks : public NimBLEScanCallbacks
        {
        public:
            explicit Callbacks(BleScanApp &app) : app_(app) {}

            void onResult(const NimBLEAdvertisedDevice *d) override
            {
                if (!d)
                    return;

                BleScanEntry e;
                e.addr = d->getAddress().toString();
                e.name = d->getName();
                e.rssi = d->getRSSI();
                app_.upsert_(e);
            }

        private:
            BleScanApp &app_;
        };

        bool scanning_ = false;
        uint32_t last_restart_ms_ = 0;
        size_t selected_ = 0;
        std::vector<BleScanEntry> entries_;
        std::vector<std::string> lines_;
        Callbacks cb_{*this};

        void upsert_(const BleScanEntry &e)
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
            scan->setScanCallbacks(&cb_, true);
            scan->setActiveScan(true);
            scan->setInterval(45);
            scan->setWindow(15);
            scan->start(5, false);
            scanning_ = true;
            last_restart_ms_ = millis();
        }

        void stop_scan_()
        {
            NimBLEDevice::getScan()->stop();
            scanning_ = false;
        }
    };
}
