#pragma once

#include "../../app.h"
#include "../../display.h"
#include "../../ui.h"
#include "../../log.h"

#include "pn532_common.h"

#include <vector>

namespace flopper
{
    class Pn532MifareClassicApp : public App
    {
    public:
        Pn532MifareClassicApp() : App("Mifare Classic") {}

        void on_enter() override
        {
            ok_ = pn532::ensure_init();
            selected_ = 0;
            block_ = 4;
            key_b_ = false;
            uri_idx_ = 0;
            confirm_.close();
            confirm_action_ = Action::None;
            last_read_.clear();
            clear_tag_();
            last_poll_ms_ = 0;
            log::line("PN532", "mifare enter");
        }

        void on_exit() override
        {
            log::line("PN532", "mifare exit");
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
                flopper::ui::draw_status(Display::get_instance(), "Mifare Classic (not found)");
                Display::get_instance().fill_rect(0, 30, 240, 210, flopper::ui::BACKGROUND_COLOR);
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 40, "No PN532 detected", flopper::ui::TEXT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
                Display::get_instance().draw_text(flopper::ui::MARGIN_X, 60, "LEFT=back", flopper::ui::ACCENT_COLOR, 2, flopper::ui::BACKGROUND_COLOR);
                return;
            }

            if (confirm_.active)
            {
                confirm_.draw(Display::get_instance());
                return;
            }

            flopper::ui::draw_status(Display::get_instance(), has_tag_ ? "Mifare Classic" : "Mifare Classic (no tag)");

            lines_.clear();
            items_.clear();

            lines_.push_back(std::string("UID: ") + (has_tag_ ? uid_s_ : std::string("(none)")));

            {
                char buf[48];
                snprintf(buf, sizeof(buf), "Block: %u", (unsigned)block_);
                lines_.push_back(buf);
            }

            lines_.push_back(std::string("Key: ") + (key_b_ ? "B" : "A"));
            lines_.push_back(std::string("Read: ") + (last_read_.size() ? last_read_ : std::string("(press CENTER)")));
            lines_.push_back("Write test pattern");
            lines_.push_back("Format NDEF");

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

            if (flopper::ui::apply_list_nav(e, selected_, 7))
                return;
            if (e == InputEvent::RIGHT)
            {
                if (selected_ == 1)
                {
                    block_++;
                }
                else if (selected_ == 2)
                {
                    key_b_ = !key_b_;
                }
                else if (selected_ == 6)
                {
                    uri_idx_ = (uri_idx_ + 1) % kUriCount;
                }
                return;
            }
            if (e == InputEvent::CENTER)
            {
                if (selected_ == 1)
                {
                    block_++;
                    return;
                }
                if (selected_ == 2)
                {
                    key_b_ = !key_b_;
                    return;
                }
                if (selected_ == 3)
                {
                    read_block_();
                    return;
                }
                if (selected_ == 4)
                {
                    confirm_action_ = Action::WriteTestPattern;
                    confirm_.open(confirm_message_());
                    return;
                }
                if (selected_ == 5)
                {
                    confirm_action_ = Action::FormatNdef;
                    confirm_.open(confirm_message_());
                    return;
                }
                if (selected_ == 6)
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
            FormatNdef,
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

        uint8_t block_ = 4;
        bool key_b_ = false;
        size_t uri_idx_ = 0;

        std::string last_read_;
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

        bool auth_()
        {
            if (!has_tag_)
                return false;

            uint8_t keya[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            uint8_t keyb[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            uint8_t *key = key_b_ ? keyb : keya;

            const uint8_t keyno = key_b_ ? 1 : 0;
            const uint8_t res = pn532::dev.mifareclassic_AuthenticateBlock(uid_, uid_len_, block_, keyno, key);
            if (!res)
            {
                flopper::log::printf("PN532", "auth failed uid=%s block=%u key=%s", uid_s_.c_str(), (unsigned)block_, key_b_ ? "B" : "A");
                return false;
            }
            return true;
        }

        void read_block_()
        {
            last_read_.clear();
            if (!auth_())
            {
                last_read_ = "auth failed";
                return;
            }
            uint8_t data[16] = {0};
            const uint8_t ok = pn532::dev.mifareclassic_ReadDataBlock(block_, data);
            if (!ok)
            {
                last_read_ = "read failed";
                return;
            }
            last_read_ = pn532::bytes_to_hex(data, sizeof(data));
            flopper::log::printf("PN532", "read block %u: %s", (unsigned)block_, last_read_.c_str());
        }

        void write_test_pattern_()
        {
            last_read_.clear();
            if (!auth_())
            {
                last_read_ = "auth failed";
                return;
            }
            uint8_t data[16] = {0};
            data[0] = 'F';
            data[1] = 'L';
            data[2] = 'O';
            data[3] = 'P';
            data[4] = (uint8_t)block_;
            data[5] = key_b_ ? 0xB0 : 0xA0;
            for (int i = 6; i < 16; i++)
                data[i] = (uint8_t)(0xA5 ^ i);

            const uint8_t ok = pn532::dev.mifareclassic_WriteDataBlock(block_, data);
            last_read_ = ok ? "write ok" : "write failed";
            flopper::log::printf("PN532", "write block %u %s", (unsigned)block_, ok ? "ok" : "failed");
        }

        void format_ndef_()
        {
            const uint8_t ok = pn532::dev.mifareclassic_FormatNDEF();
            last_read_ = ok ? "format ok" : "format failed";
            flopper::log::printf("PN532", "format ndef %s", ok ? "ok" : "failed");
        }

        void write_ndef_uri_()
        {
            const UriOpt &u = kUris[uri_idx_];
            const uint8_t ok = pn532::dev.mifareclassic_WriteNDEFURI(1, u.prefix, u.url);
            last_read_ = ok ? "uri write ok" : "uri write failed";
            flopper::log::printf("PN532", "write uri %s %s", u.label, ok ? "ok" : "failed");
        }

        const char *confirm_message_() const
        {
            if (!has_tag_)
                return "No tag present";
            switch (confirm_action_)
            {
            case Action::WriteTestPattern:
                return "Write test pattern?";
            case Action::FormatNdef:
                return "Format tag as NDEF?";
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
            case Action::FormatNdef:
                format_ndef_();
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
