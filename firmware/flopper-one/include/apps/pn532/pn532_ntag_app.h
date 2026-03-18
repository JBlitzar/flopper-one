#pragma once

#include "../../app.h"
#include "../../display.h"
#include "../../ui.h"
#include "../../log.h"

#include "pn532_common.h"

#include <vector>
#include <cstring>

namespace flopper
{
    class Pn532NtagApp : public App
    {
    public:
        Pn532NtagApp() : App("NTAG/Ultralight") {}

        void on_enter() override
        {
            ok_ = pn532::ensure_init();
            selected_ = 0;
            page_ = 4;
            uri_idx_ = 0;
            confirm_.close();
            confirm_action_ = Action::None;
            last_page_.clear();
            clear_tag_();
            last_poll_ms_ = 0;
            log::line("PN532", "ntag enter");
        }

        void on_exit() override
        {
            log::line("PN532", "ntag exit");
        }

        void tick() override
        {
            if (ok_ && millis() - last_poll_ms_ > 120)
            {
                poll_tag_();
                last_poll_ms_ = millis();
            }
            draw();
        }

        void draw() override
        {
            if (!ok_)
            {
                flopper::ui::draw_status(Display::get_instance(), "NTAG/Ultralight (not found)");
                Display::get_instance().fill_rect(0, 30, 240, 210, TFT_BLACK);
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, "No PN532 detected", TFT_WHITE, 2, TFT_BLACK);
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, "LEFT=back", TFT_CYAN, 2, TFT_BLACK);
                return;
            }

            if (confirm_.active)
            {
                confirm_.draw(Display::get_instance());
                return;
            }

            flopper::ui::draw_status(Display::get_instance(), has_tag_ ? "NTAG/Ultralight" : "NTAG/Ultralight (no tag)");

            lines_.clear();
            items_.clear();

            lines_.push_back(std::string("UID: ") + (has_tag_ ? uid_s_ : std::string("(none)")));

            {
                char buf[48];
                snprintf(buf, sizeof(buf), "Page: %u", (unsigned)page_);
                lines_.push_back(buf);
            }

            lines_.push_back(std::string("Read: ") + (last_page_.size() ? last_page_ : std::string("(press CENTER)")));
            lines_.push_back("Write test pattern");

            {
                char buf[96];
                snprintf(buf, sizeof(buf), "Write NDEF URI: %s", kUris[uri_idx_].label);
                lines_.push_back(buf);
            }

            for (auto &s : lines_)
                items_.push_back(s.c_str());
            flopper::ui::draw_list(Display::get_instance(), items_, flopper::ui::clamp_index(selected_, items_.size()));
        }

        void on_input(InputEvent e) override
        {
            if (!ok_)
                return;

            if (confirm_.active)
            {
                const auto r = confirm_.on_input(e);
                if (r == flopper::ui::ConfirmResult::Yes)
                    run_confirmed_();
                if (r != flopper::ui::ConfirmResult::None)
                    confirm_action_ = Action::None;
                return;
            }

            if (e == InputEvent::LEFT)
            {
                exit();
                return;
            }

            if (flopper::ui::apply_list_nav(e, selected_, 5))
                return;
            if (e == InputEvent::RIGHT)
            {
                if (selected_ == 1)
                {
                    page_++;
                }
                else if (selected_ == 4)
                {
                    uri_idx_ = (uri_idx_ + 1) % kUriCount;
                }
                return;
            }
            if (e == InputEvent::CENTER)
            {
                if (selected_ == 1)
                {
                    page_++;
                    return;
                }
                if (selected_ == 2)
                {
                    read_page_();
                    return;
                }
                if (selected_ == 3)
                {
                    confirm_action_ = Action::WriteTestPattern;
                    confirm_.open(confirm_message_());
                    return;
                }
                if (selected_ == 4)
                {
                    confirm_action_ = Action::WriteNdefUri;
                    confirm_.open(confirm_message_());
                    return;
                }
            }
        }

    private:
        enum class Action
        {
            None,
            WriteTestPattern,
            WriteNdefUri,
        };

        struct UriOpt
        {
            const char *label;
            uint8_t prefix;
            const char *url;
        };

        inline static constexpr size_t kUriCount = 4;
        inline static constexpr UriOpt kUris[kUriCount] = {
            {"https://example.com", NDEF_URIPREFIX_HTTPS, "example.com"},
            {"https://github.com", NDEF_URIPREFIX_HTTPS, "github.com"},
            {"http://example.com", NDEF_URIPREFIX_HTTP, "example.com"},
            {"https://adafruit.com", NDEF_URIPREFIX_HTTPS, "adafruit.com"},
        };

        bool ok_ = false;
        uint32_t last_poll_ms_ = 0;
        bool has_tag_ = false;
        uint8_t uid_[7] = {0};
        uint8_t uid_len_ = 0;
        std::string uid_s_;
        uint32_t last_seen_ms_ = 0;

        uint8_t page_ = 4;
        size_t uri_idx_ = 0;

        std::string last_page_;
        std::vector<std::string> lines_;
        std::vector<const char *> items_;
        size_t selected_ = 0;

        flopper::ui::ConfirmDialog confirm_;
        Action confirm_action_ = Action::None;

        void clear_tag_()
        {
            has_tag_ = false;
            uid_len_ = 0;
            uid_s_.clear();
            memset(uid_, 0, sizeof(uid_));
        }

        void poll_tag_()
        {
            uint8_t uid[7] = {0};
            uint8_t uid_len = 0;
            const bool ok = pn532::dev.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uid_len, 10);
            if (ok)
            {
                has_tag_ = true;
                memcpy(uid_, uid, uid_len);
                uid_len_ = uid_len;
                uid_s_ = pn532::uid_to_string(uid, uid_len);
                last_seen_ms_ = millis();
            }
            else if (has_tag_ && millis() - last_seen_ms_ > 1000)
            {
                clear_tag_();
            }
        }

        void read_page_()
        {
            last_page_.clear();
            if (!has_tag_)
            {
                last_page_ = "no tag";
                return;
            }
            uint8_t data[4] = {0};
            const uint8_t ok = pn532::dev.ntag2xx_ReadPage(page_, data);
            if (!ok)
            {
                last_page_ = "read failed";
                return;
            }
            last_page_ = pn532::bytes_to_hex(data, sizeof(data));
            flopper::log::printf("PN532", "ntag read page %u: %s", (unsigned)page_, last_page_.c_str());
        }

        void write_test_pattern_()
        {
            if (!has_tag_)
            {
                last_page_ = "no tag";
                return;
            }
            uint8_t data[4] = {0xDE, 0xAD, 0xBE, (uint8_t)(0xEF ^ page_)};
            const uint8_t ok = pn532::dev.ntag2xx_WritePage(page_, data);
            last_page_ = ok ? "write ok" : "write failed";
            flopper::log::printf("PN532", "ntag write page %u %s", (unsigned)page_, ok ? "ok" : "failed");
        }

        void write_ndef_uri_()
        {
            const UriOpt &u = kUris[uri_idx_];
            char url_buf[64];
            strncpy(url_buf, u.url, sizeof(url_buf));
            url_buf[sizeof(url_buf) - 1] = 0;
            const uint8_t ok = pn532::dev.ntag2xx_WriteNDEFURI(u.prefix, url_buf, (uint8_t)strlen(url_buf));
            last_page_ = ok ? "uri write ok" : "uri write failed";
            flopper::log::printf("PN532", "ntag write uri %s %s", u.label, ok ? "ok" : "failed");
        }

        const char *confirm_message_() const
        {
            if (!has_tag_)
                return "No tag present";
            switch (confirm_action_)
            {
            case Action::WriteTestPattern:
                return "Write test pattern?";
            case Action::WriteNdefUri:
                return "Write NDEF URI?";
            default:
                return "Confirm?";
            }
        }

        void run_confirmed_()
        {
            if (!has_tag_)
                return;
            switch (confirm_action_)
            {
            case Action::WriteTestPattern:
                write_test_pattern_();
                break;
            case Action::WriteNdefUri:
                write_ndef_uri_();
                break;
            default:
                break;
            }
        }
    };
}
