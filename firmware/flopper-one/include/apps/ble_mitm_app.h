#pragma once

#include "../app.h"
#include "../display.h"
#include "../ui.h"
#include "../log.h"
#include "../ble/ble_common.h"

#include <NimBLEDevice.h>

namespace flopper
{
    class BleMitmApp : public App
    {
    public:
        BleMitmApp() : App("BLE MITM") {}

        void on_enter() override
        {
            flopper::ble::ensure_init("flopper-one");
            state_ = State::Scanning;
            entries_.clear();
            selected_ = 0;
            start_scan_();
            log::line("BLE", "mitm enter");
        }

        void on_exit() override
        {
            stop_all_();
            log::line("BLE", "mitm exit");
        }

        void tick() override
        {
            draw();
        }

        void draw() override
        {
            const char *hdr = nullptr;
            switch (state_)
            {
            case State::Scanning:
                hdr = "MITM: pick target";
                break;
            case State::ConnectingPeripheral:
                hdr = "MITM: connecting...";
                break;
            case State::Proxying:
                hdr = "MITM: proxying";
                break;
            case State::Error:
                hdr = "MITM: error";
                break;
            }
            flopper::ui::draw_status(Display::get_instance(), hdr);

            if (state_ == State::Scanning)
            {
                lines_.clear();
                std::vector<const char *> items;
                lines_.reserve(entries_.size());
                items.reserve(entries_.size());
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
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, "LEFT=stop/back", TFT_CYAN, 2, TFT_BLACK);
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
                connect_and_proxy_(entries_[selected_]);
            }
        }

    private:

        enum class State
        {
            Scanning,
            ConnectingPeripheral,
            Proxying,
            Error
        };

        struct Entry
        {
            std::string addr;
            uint8_t addr_type = 0;
            std::string name;
            int rssi;
        };

        class ScanCallbacks : public NimBLEScanCallbacks
        {
        public:
            explicit ScanCallbacks(BleMitmApp &app) : app_(app) {}
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
            BleMitmApp &app_;
        };

        class ProxyServerCallbacks : public NimBLEServerCallbacks
        {
        public:
            explicit ProxyServerCallbacks(BleMitmApp &app) : app_(app) {}
            void onDisconnect(NimBLEServer *, NimBLEConnInfo &, int) override
            {
                NimBLEDevice::startAdvertising();
                log::line("BLE", "proxy host disconnected");
            }

        private:
            BleMitmApp &app_;
        };

        class CharCallbacks : public NimBLECharacteristicCallbacks
        {
        public:
            CharCallbacks(BleMitmApp &app, NimBLECharacteristic *local, NimBLERemoteCharacteristic *remote)
                : app_(app), local_(local), remote_(remote) {}

            void onRead(NimBLECharacteristic *c, NimBLEConnInfo &) override
            {
                if (!remote_ || !c)
                    return;
                if (!remote_->canRead())
                    return;
                const std::string v = remote_->readValue();
                c->setValue((uint8_t *)v.data(), v.size());
            }

            void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &) override
            {
                if (!c || !remote_)
                    return;

                const std::string v = c->getValue();
                const bool response = remote_->canWrite();
                remote_->writeValue((uint8_t *)v.data(), v.size(), response);
                log::printf("MITM", "host->periph %s %u bytes", remote_->getUUID().toString().c_str(), (unsigned)v.size());
            }

        private:
            BleMitmApp &app_;
            NimBLECharacteristic *local_;
            NimBLERemoteCharacteristic *remote_;
        };

        State state_ = State::Scanning;
        std::string status_ = "";
        size_t selected_ = 0;

        std::vector<Entry> entries_;
        std::vector<std::string> lines_;
        ScanCallbacks scan_cb_{*this};
        ProxyServerCallbacks server_cb_{*this};
        std::vector<CharCallbacks *> char_cbs_;

        NimBLEClient *client_ = nullptr;

        NimBLEServer *server_ = nullptr;
        std::vector<NimBLECharacteristic *> local_notify_chars_;

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
            if (entries_.size() > 20)
                entries_.erase(entries_.begin());
        }

        void start_scan_()
        {
            NimBLEScan *scan = NimBLEDevice::getScan();
            scan->setScanCallbacks(&scan_cb_, true);
            scan->setActiveScan(true);
            scan->start(5, false);
        }

        void stop_scan_()
        {
            NimBLEDevice::getScan()->stop();
        }

        void connect_and_proxy_(const Entry &target)
        {
            stop_scan_();
            state_ = State::ConnectingPeripheral;
            status_ = std::string("connecting ") + target.addr;

            client_ = NimBLEDevice::createClient();
            if (!client_->connect(NimBLEAddress(target.addr, target.addr_type)))
            {
                state_ = State::Error;
                status_ = "connect failed";
                return;
            }

            if (!build_proxy_(target))
            {
                state_ = State::Error;
                if (status_.empty())
                    status_ = "proxy build failed";
                return;
            }
            state_ = State::Proxying;
            status_ = "proxy ready";
        }

        bool build_proxy_(const Entry &target)
        {
            // Clone advertised name where possible.
            NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
            adv->stop();
            adv->reset();
            if (target.name.size())
                adv->setName(target.name);

            server_ = NimBLEDevice::createServer();
            server_->setCallbacks(&server_cb_);

            // Discover & mirror services/characteristics by UUID.
            const auto svcs = client_->getServices(true);
            if (svcs.empty())
            {
                status_ = "no services";
                return false;
            }

            bool adv_uuid_added = false;
            for (auto *rs : svcs)
            {
                if (!rs)
                    continue;

                NimBLEService *ls = server_->createService(rs->getUUID());
                if (!ls)
                    continue;

                if (!adv_uuid_added)
                {
                    adv->addServiceUUID(rs->getUUID());
                    adv_uuid_added = true;
                }

                log::printf("MITM", "svc %s", rs->getUUID().toString().c_str());

                const auto chars = rs->getCharacteristics(true);
                for (auto *rc : chars)
                {
                    if (!rc)
                        continue;

                    uint32_t props = 0;
                    if (rc->canRead())
                        props |= NIMBLE_PROPERTY::READ;
                    if (rc->canWrite())
                        props |= NIMBLE_PROPERTY::WRITE;
                    if (rc->canWriteNoResponse())
                        props |= NIMBLE_PROPERTY::WRITE_NR;
                    if (rc->canNotify())
                        props |= NIMBLE_PROPERTY::NOTIFY;
                    if (rc->canIndicate())
                        props |= NIMBLE_PROPERTY::INDICATE;

                    NimBLECharacteristic *lc = ls->createCharacteristic(rc->getUUID(), props);
                    if (!lc)
                        continue;

                    auto *cb = new CharCallbacks(*this, lc, rc);
                    char_cbs_.push_back(cb);
                    lc->setCallbacks(cb);

                    log::printf("MITM", " chr %s props=0x%lx", rc->getUUID().toString().c_str(), (unsigned long)props);

                    if (rc->canNotify() || rc->canIndicate())
                    {
                        // Subscribe and forward to host (if subscribed).
                        rc->subscribe(true,
                                      [this, lc](NimBLERemoteCharacteristic *r, uint8_t *data, size_t len, bool) {
                                          if (!lc)
                                              return;
                                          lc->setValue(data, len);
                                          lc->notify();
                                          if (r)
                                              log::printf("MITM", "periph->host %s %u bytes", r->getUUID().toString().c_str(), (unsigned)len);
                                          else
                                              log::printf("MITM", "periph->host %u bytes", (unsigned)len);
                                      });
                        local_notify_chars_.push_back(lc);
                    }
                }

                ls->start();
            }

            adv->enableScanResponse(true);
            adv->start();
            return true;
        }

        void stop_proxy_()
        {
            NimBLEDevice::getAdvertising()->stop();
            for (auto *cb : char_cbs_)
                delete cb;
            char_cbs_.clear();
            local_notify_chars_.clear();
        }

        void stop_all_()
        {
            stop_scan_();
            stop_proxy_();
            if (client_)
            {
                if (client_->isConnected())
                    client_->disconnect();
                NimBLEDevice::deleteClient(client_);
                client_ = nullptr;
            }
            server_ = nullptr;
            state_ = State::Scanning;
        }
    };
}
