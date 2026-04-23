#pragma once

#include "../../app.h"
#include "../../display.h"
#include "../../ui.h"
#include "../../log.h"

#include "pn532_common.h"

#include <vector>

namespace flopper
{
    struct Pn532SeenTag
    {
        std::string uid;
        uint8_t uid_len = 0;
        uint32_t last_seen_ms = 0;
    };

    class Pn532ScanApp : public App
    {
    public:
        Pn532ScanApp() : App("PN532 Scan") {}

        void on_enter() override
        {
            ok_ = false;
            first_frame_ = true;
            selected_ = 0;
            show_detail_ = false;
            tags_.clear();
            last_poll_ms_ = 0;
            log::line("PN532", "scan enter");
        }

        void on_exit() override
        {
            log::line("PN532", "scan exit");
        }

        void tick() override
        {
            if (first_frame_)
            {
                first_frame_ = false;
                draw();
                return;
            }

            if (!ok_)
            {
                ok_ = (pn532::tick_init() == pn532::InitState::Ready);
                draw();
                return;
            }

            if (millis() - last_poll_ms_ > 120)
            {
                poll_();
                last_poll_ms_ = millis();
            }
            draw();
        }

        void draw() override
        {
            if (!ok_)
            {
                char title[64];
                snprintf(title, sizeof(title), "PN532 Scan (%s)", pn532::init_state_label(pn532::init_state));
                flopper::ui::draw_status(Display::get_instance(), title);
                Display::get_instance().fill_rect(0, 30, 240, 210, flopper::ui::BACKGROUND_COLOR);
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40,
                                                  pn532::init_state == pn532::InitState::InProgress || pn532::init_state == pn532::InitState::NotStarted
                                                      ? "Initializing..."
                                                      : "No PN532 detected",
                                                  flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, "LEFT=back", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
                return;
            }

            if (show_detail_)
            {
                draw_detail_();
                return;
            }

            flopper::ui::draw_status(Display::get_instance(), "PN532 Scan (RIGHT=details)");

            lines_.clear();
            items_.clear();
            for (auto &t : tags_)
            {
                char buf[96];
                snprintf(buf, sizeof(buf), "%ub %s", (unsigned)t.uid_len, t.uid.c_str());
                lines_.push_back(buf);
            }

            if (lines_.empty())
                lines_.push_back("(present tag to scan)");

            for (auto &s : lines_)
                items_.push_back(s.c_str());
            flopper::ui::draw_list(Display::get_instance(), items_, flopper::ui::clamp_index(selected_, items_.size()));
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
            if (!ok_)
                return;

            if (flopper::ui::apply_list_nav(e, selected_, tags_.size()))
                return;
            if (e == InputEvent::RIGHT)
            {
                if (!tags_.empty())
                    show_detail_ = true;
                return;
            }
            if (e == InputEvent::CENTER)
            {
                tags_.clear();
                selected_ = 0;
                return;
            }
        }

    private:
        bool ok_ = false;
        bool first_frame_ = true;
        uint32_t last_poll_ms_ = 0;
        std::vector<Pn532SeenTag> tags_;
        std::vector<std::string> lines_;
        std::vector<const char *> items_;
        size_t selected_ = 0;
        bool show_detail_ = false;

        void poll_()
        {
            uint8_t uid[7] = {0};
            uint8_t uid_len = 0;
            const bool ok = pn532::dev.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uid_len, 10);
            if (!ok)
                return;

            const std::string uid_s = pn532::uid_to_string(uid, uid_len);
            const uint32_t now = millis();

            for (auto &t : tags_)
            {
                if (t.uid == uid_s)
                {
                    t.uid_len = uid_len;
                    t.last_seen_ms = now;
                    return;
                }
            }

            Pn532SeenTag t;
            t.uid = uid_s;
            t.uid_len = uid_len;
            t.last_seen_ms = now;
            tags_.insert(tags_.begin(), t);
            if (tags_.size() > 12)
                tags_.pop_back();

            flopper::log::printf("PN532", "tag %ub %s", (unsigned)uid_len, uid_s.c_str());
        }

        void draw_detail_()
        {
            const Pn532SeenTag &t = tags_[selected_ < tags_.size() ? selected_ : 0];
            flopper::ui::draw_status(Display::get_instance(), "PN532 Tag Details");
            Display::get_instance().fill_rect(0, 30, 240, 210, flopper::ui::BACKGROUND_COLOR);

            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, t.uid.c_str(), flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            char buf[64];
            snprintf(buf, sizeof(buf), "uid_len=%u", (unsigned)t.uid_len);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, buf, flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 78, pn532::guess_tag_type(t.uid_len), flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);

            snprintf(buf, sizeof(buf), "seen %lus ago", (unsigned long)((millis() - t.last_seen_ms) / 1000));
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 100, buf, flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);

            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 140, "CENTER=clear", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
            Display::get_instance().draw_text(flopper::ui::MARGIN_X, 158, "LEFT=back", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
        }
    };
}
