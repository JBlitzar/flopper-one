#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <chrono>
#include <thread>
#include <random>

// Basic Arduino-compatible types/macros used by the desktop UI shim.
using byte = uint8_t;
using boolean = bool;

// Flash string helpers (no-op on desktop).
class __FlashStringHelper;
#ifndef F
#define F(x) (x)
#endif

// Math helpers expected by some Arduino libraries.
#ifndef PI
#define PI 3.1415926535897932384626433832795
#endif
#ifndef DEG_TO_RAD
#define DEG_TO_RAD (PI / 180.0)
#endif
#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0 / PI)
#endif

inline float radians(float deg) { return deg * (float)DEG_TO_RAD; }
inline float degrees(float rad) { return rad * (float)RAD_TO_DEG; }

#ifndef PROGMEM
#define PROGMEM
#endif

template <typename T>
inline T pgm_read_impl(const T *addr)
{
    return *addr;
}

#ifndef pgm_read_byte
#define pgm_read_byte(addr) pgm_read_impl((const uint8_t *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) pgm_read_impl((const uint16_t *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) pgm_read_impl((addr))
#endif

#ifndef PGM_P
#define PGM_P const char *
#endif

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

#ifndef ICACHE_RAM_ATTR
#define ICACHE_RAM_ATTR
#endif

#ifndef INLINE
#define INLINE inline
#endif

// Pin modes / levels
static constexpr int HIGH = 1;
static constexpr int LOW = 0;
static constexpr int INPUT = 0;
static constexpr int OUTPUT = 1;
static constexpr int INPUT_PULLUP = 2;

inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline int digitalRead(int) { return HIGH; }

inline void delay(uint32_t ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void delayMicroseconds(uint32_t) {}

inline uint32_t millis()
{
    static const auto start = std::chrono::steady_clock::now();
    const auto now = std::chrono::steady_clock::now();
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

inline void yield() {}

inline uint32_t digitalPinToBitMask(int pin)
{
    if (pin < 0)
        return 0;
    if (pin >= 32)
        return 0;
    return (1u << (uint32_t)pin);
}

// Minimal Arduino String used by library String overloads.
class String
{
public:
    String() = default;
    String(const char *s) : s_(s ? s : "") {}
    String(const std::string &s) : s_(s) {}

    String &operator=(const char *s)
    {
        s_ = s ? s : "";
        return *this;
    }

    bool operator==(const char *rhs) const
    {
        return s_ == (rhs ? rhs : "");
    }

    bool operator!=(const char *rhs) const
    {
        return !(*this == rhs);
    }

    size_t length() const { return s_.size(); }
    const char *c_str() const { return s_.c_str(); }

    void toCharArray(char *buf, unsigned int bufsize) const
    {
        if (!buf || bufsize == 0)
            return;
        const size_t n = (s_.size() < (size_t)(bufsize - 1)) ? s_.size() : (size_t)(bufsize - 1);
        std::memcpy(buf, s_.data(), n);
        buf[n] = '\0';
    }

private:
    std::string s_;
};

// Arduino helpers expected by common Arduino libraries.
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

inline long random(long max_exclusive)
{
    if (max_exclusive <= 0)
        return 0;
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<long> dist(0, max_exclusive - 1);
    return dist(rng);
}

inline long random(long min_inclusive, long max_exclusive)
{
    if (max_exclusive <= min_inclusive)
        return min_inclusive;
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<long> dist(min_inclusive, max_exclusive - 1);
    return dist(rng);
}

inline char *ltoa(long value, char *str, int base)
{
    if (!str || base < 2 || base > 36)
        return str;

    char *p = str;
    bool neg = false;
    unsigned long v;
    if (value < 0)
    {
        neg = true;
        v = (unsigned long)(-value);
    }
    else
    {
        v = (unsigned long)value;
    }

    char buf[64];
    size_t i = 0;
    do
    {
        const unsigned digit = (unsigned)(v % (unsigned long)base);
        buf[i++] = (char)(digit < 10 ? ('0' + digit) : ('a' + (digit - 10)));
        v /= (unsigned long)base;
    } while (v && i < sizeof(buf));

    if (neg)
        *p++ = '-';
    while (i)
        *p++ = buf[--i];
    *p = '\0';
    return str;
}
