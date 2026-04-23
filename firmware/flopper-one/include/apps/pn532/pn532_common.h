#pragma once

#include "../../flopper_pins.h"
#include "../../log.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

#include <string>

namespace flopper::pn532
{
    // Most PN532 breakouts work fine over I2C without wiring IRQ/RESET.
    // Passing 0xFF maps to internal -1 and disables those features.
    inline constexpr uint8_t kIrqPin = 0xFF;
    inline constexpr uint8_t kResetPin = 0xFF;

    inline Adafruit_PN532 dev{kIrqPin, kResetPin, &Wire};
    enum class InitState : uint8_t
    {
        NotStarted,
        InProgress,
        Ready,
        Failed,
    };

    inline InitState init_state = InitState::NotStarted;
    inline bool present = false;
    inline bool wire_ready = false;
    inline bool dev_begun = false;
    inline bool logged_bus_once = false;
    inline uint32_t last_attempt_ms = 0;
    inline uint8_t attempts = 0;
    inline uint8_t last_probe_err = 0xFF;
    inline uint32_t firmware_version = 0;

    inline void log_i2c_scan_()
    {
        uint8_t found = 0;
        std::string addrs;
        for (uint8_t addr = 1; addr < 127; addr++)
        {
            Wire.beginTransmission(addr);
            const uint8_t err = Wire.endTransmission();
            if (err == 0)
            {
                char buf[8];
                snprintf(buf, sizeof(buf), "0x%02X", (unsigned)addr);
                if (!addrs.empty())
                    addrs += ' ';
                addrs += buf;
                found++;
            }
            delay(2);
        }
        if (found)
            flopper::log::printf("PN532", "I2C devices (%u): %s", (unsigned)found, addrs.c_str());
        else
            flopper::log::line("PN532", "I2C scan: no devices found");
    }

    inline void log_pn532_probe_()
    {
        // Probe the PN532 I2C address directly. This does not guarantee PN532 is awake,
        // but it quickly distinguishes 'not on bus' vs 'protocol/timing'.
        Wire.beginTransmission(PN532_I2C_ADDRESS);
        const uint8_t err = Wire.endTransmission();
        last_probe_err = err;
        flopper::log::printf("PN532", "probe addr 0x%02X endTransmission=%u", (unsigned)PN532_I2C_ADDRESS, (unsigned)err);
    }

    inline bool is_firmware_version_plausible_(uint32_t version)
    {
        const uint8_t chip = (uint8_t)((version >> 24) & 0xFF);
        const uint8_t fw_major = (uint8_t)((version >> 16) & 0xFF);
        const uint8_t fw_minor = (uint8_t)((version >> 8) & 0xFF);

        // Typical Adafruit PN532 firmware version looks like 0x32xxxxxx.
        // If we read 0x00 or 0xFF in these fields it's usually an I2C glitch.
        if (chip < 0x30 || chip > 0x34)
            return false;
        if (fw_major == 0x00 || fw_major == 0xFF)
            return false;
        if (fw_minor == 0xFF)
            return false;
        return true;
    }

    inline const char *init_state_label(InitState s)
    {
        switch (s)
        {
        case InitState::NotStarted:
            return "not started";
        case InitState::InProgress:
            return "initializing";
        case InitState::Ready:
            return "ready";
        default:
            return "not found";
        }
    }

    inline void reset_init()
    {
        init_state = InitState::NotStarted;
        present = false;
        wire_ready = false;
        dev_begun = false;
        logged_bus_once = false;
        last_attempt_ms = 0;
        attempts = 0;
        last_probe_err = 0xFF;
        firmware_version = 0;
    }

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

    inline InitState tick_init()
    {
        if (init_state == InitState::Ready)
            return init_state;

        const uint32_t now = millis();
        if (!wire_ready)
        {
            Wire.begin(flopper::pins::PN532_SDA, flopper::pins::PN532_SCL);
            // PN532 I2C can be finicky; lower clock improves reliability on some modules.
            Wire.setClock(50000);
            // ESP32 Wire has a transaction timeout; PN532 may clock-stretch during wake/SAMConfig.
            // Keep this relatively low so failures don't freeze the UI.
            Wire.setTimeOut(80);
            wire_ready = true;
        }

        if (!logged_bus_once)
        {
            flopper::log::printf("PN532", "I2C SDA=%u SCL=%u (irq=%d rst=%d)",
                                 (unsigned)flopper::pins::PN532_SDA,
                                 (unsigned)flopper::pins::PN532_SCL,
                                 (int8_t)kIrqPin,
                                 (int8_t)kResetPin);
            logged_bus_once = true;
        }

        // Backoff so we don't repeatedly block the UI if the device isn't present.
        // Short interval keeps init responsive while staying lightweight.
        constexpr uint32_t kAttemptEveryMs = 150;
        if (attempts && (now - last_attempt_ms) < kAttemptEveryMs)
            return init_state;

        last_attempt_ms = now;
        attempts++;
        init_state = InitState::InProgress;

        // Fast address probe first.
        Wire.beginTransmission(PN532_I2C_ADDRESS);
        const uint8_t probe_err = Wire.endTransmission();
        last_probe_err = probe_err;
        if (probe_err != 0)
        {
            present = false;
            // Occasionally dump an I2C scan to aid debugging without adding constant latency.
            if (attempts == 1 || (attempts % 10) == 0)
            {
                flopper::log::printf("PN532", "probe err=%u (attempt %u)", (unsigned)probe_err, (unsigned)attempts);
                log_i2c_scan_();
            }
            // Keep trying; treat as not found for UI.
            init_state = InitState::Failed;
            return init_state;
        }

        // Only call begin() once; it does reset+wakeup+SAMConfig internally and can be slow.
        if (!dev_begun)
        {
            if (!dev.begin())
            {
                present = false;
                if (attempts == 1 || (attempts % 10) == 0)
                    flopper::log::line("PN532", "begin failed");
                init_state = InitState::Failed;
                return init_state;
            }
            dev_begun = true;
        }

        const uint32_t version = dev.getFirmwareVersion();
        if (!is_firmware_version_plausible_(version))
        {
            present = false;
            if (attempts == 1 || (attempts % 10) == 0)
            {
                flopper::log::printf("PN532", "invalid firmware version 0x%08lX", (unsigned long)version);
            }

            // If the device ACKs but returns junk, try a periodic SAMConfig to re-sync.
            if ((attempts % 5) == 0)
                (void)dev.SAMConfig();

            init_state = InitState::Failed;
            return init_state;
        }

        firmware_version = version;
        present = true;
        init_state = InitState::Ready;

        dev.SAMConfig();
        dev.setPassiveActivationRetries(0xFF);
        flopper::log::printf("PN532", "found PN5%02lX fw %lu.%lu",
                             (unsigned long)((version >> 24) & 0xFF),
                             (unsigned long)((version >> 16) & 0xFF),
                             (unsigned long)((version >> 8) & 0xFF));
        return init_state;
    }

    inline bool ensure_init()
    {
        // Backwards-compatible wrapper. Prefer calling tick_init() from App::tick()
        // to avoid blocking the UI on slow I2C operations.
        return tick_init() == InitState::Ready;
    }
}
