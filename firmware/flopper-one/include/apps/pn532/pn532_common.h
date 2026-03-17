#pragma once

#include "../../flopper_pins.h"
#include "../../log.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

#include <string>

namespace flopper::pn532
{
    inline constexpr uint8_t kIrqPin = 22;
    inline constexpr uint8_t kResetPin = 21;

    inline Adafruit_PN532 dev{kIrqPin, kResetPin, &Wire};
    inline bool inited = false;
    inline bool present = false;

    inline std::string uid_to_string(const uint8_t *uid, uint8_t uid_len)
    {
        static const char *hex = "0123456789ABCDEF";
        std::string s;
        s.reserve(uid_len * 2 + (uid_len ? (uid_len - 1) : 0));
        for (uint8_t i = 0; i < uid_len; i++)
        {
            if (i)
                s.push_back(':');
            s.push_back(hex[(uid[i] >> 4) & 0xF]);
            s.push_back(hex[uid[i] & 0xF]);
        }
        return s;
    }

    inline std::string bytes_to_hex(const uint8_t *data, size_t len)
    {
        static const char *hex = "0123456789ABCDEF";
        std::string s;
        s.reserve(len * 2 + (len ? (len - 1) : 0));
        for (size_t i = 0; i < len; i++)
        {
            if (i)
                s.push_back(' ');
            s.push_back(hex[(data[i] >> 4) & 0xF]);
            s.push_back(hex[data[i] & 0xF]);
        }
        return s;
    }

    inline const char *guess_tag_type(uint8_t uid_len)
    {
        if (uid_len == 4)
            return "ISO14443A (4B UID)";
        if (uid_len == 7)
            return "ISO14443A (7B UID)";
        if (uid_len == 10)
            return "ISO14443A (10B UID)";
        return "ISO14443A";
    }

    inline bool ensure_init()
    {
        if (inited)
            return present;
        inited = true;

        Wire.begin(flopper::pins::PN532_SDA, flopper::pins::PN532_SCL);
        if (!dev.begin())
        {
            present = false;
            flopper::log::line("PN532", "begin failed");
            return false;
        }

        const uint32_t version = dev.getFirmwareVersion();
        if (!version)
        {
            present = false;
            flopper::log::line("PN532", "not detected");
            return false;
        }
        present = true;

        dev.SAMConfig();
        dev.setPassiveActivationRetries(0xFF);
        flopper::log::printf("PN532", "found PN5%02lX fw %lu.%lu",
                             (unsigned long)((version >> 24) & 0xFF),
                             (unsigned long)((version >> 16) & 0xFF),
                             (unsigned long)((version >> 8) & 0xFF));
        return true;
    }
}
