#pragma once

#include "../app.h"
#include "../display.h"
#include "../ui.h"
#include "../log.h"

#include <WiFi.h>

namespace flopper
{
    class WifiApSinkApp : public App
    {
    public:
        WifiApSinkApp() : App("WiFi AP Sink") {}

        void on_enter() override
        {
            start_();
            log::line("WIFI", "ap sink enter");
        }

        void on_exit() override
        {
            stop_();
            log::line("WIFI", "ap sink exit");
        }

        void tick() override
        {
            draw();
        }

        void draw() override
        {
            const char *hdr = running_ ? "WiFi AP Sink (running)" : "WiFi AP Sink (stopped)";
            flopper::ui::draw_status(Display::get_instance(), hdr);

            Display::get_instance().fill_rect(0, 30, 240, 210, TFT_BLACK);

            int y = 40;
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, y, ssid_, TFT_WHITE, 2, TFT_BLACK);
            y += 20;

            char buf[96];
            snprintf(buf, sizeof(buf), "ch %d  open", kChannel);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, y, buf, TFT_WHITE, 2, TFT_BLACK);
            y += 20;

            const IPAddress ip = WiFi.softAPIP();
            snprintf(buf, sizeof(buf), "ip %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, y, buf, TFT_WHITE, 2, TFT_BLACK);
            y += 20;

            snprintf(buf, sizeof(buf), "clients %d", (int)WiFi.softAPgetStationNum());
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, y, buf, TFT_WHITE, 2, TFT_BLACK);
            y += 30;

            Display::get_instance().draw_text(flopper::ui::MARGIN_X, y, "CENTER=start/stop", TFT_CYAN, 2, TFT_BLACK);
            y += 20;
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, y, "LEFT=back", TFT_CYAN, 2, TFT_BLACK);
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
                if (running_)
                    stop_();
                else
                    start_();
                return;
            }
        }

    private:
        static constexpr int kChannel = 6;
        static constexpr int kMaxConn = 4;

        const char *ssid_ = "flopper-one";
        bool running_ = false;

        void start_()
        {
            if (running_)
                return;

            WiFi.mode(WIFI_AP);
            WiFi.setSleep(false);

            const bool ok = WiFi.softAP(ssid_, nullptr, kChannel, 0, kMaxConn);
            running_ = ok;

            if (ok)
            {
                const IPAddress ip = WiFi.softAPIP();
                log::printf("WIFI", "ap up ssid='%s' ch=%d ip=%u.%u.%u.%u", ssid_, kChannel, ip[0], ip[1], ip[2], ip[3]);
            }
            else
            {
                log::printf("WIFI", "ap start failed ssid='%s'", ssid_);
            }
        }

        void stop_()
        {
            if (!running_)
                return;
            WiFi.softAPdisconnect(true);
            WiFi.mode(WIFI_OFF);
            running_ = false;
            log::line("WIFI", "ap down");
        }
    };
}
