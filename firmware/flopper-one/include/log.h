#pragma once

#include <Arduino.h>
#include <cstdarg>

namespace flopper::log
{
    inline void line(const char *tag, const char *msg)
    {
        Serial.printf("[%lu][%s] %s\n", (unsigned long)millis(), tag ? tag : "?", msg ? msg : "");
    }

    inline void printf(const char *tag, const char *fmt, ...)
    {
        char buf[256];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        line(tag, buf);
    }
}
