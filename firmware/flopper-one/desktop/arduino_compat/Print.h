#pragma once

#include <cstddef>
#include <cstdint>

#include "Arduino.h"

class Print
{
public:
    virtual ~Print() = default;
    virtual size_t write(uint8_t) { return 1; }

    virtual size_t write(const uint8_t *buffer, size_t size)
    {
        if (!buffer)
            return 0;
        size_t n = 0;
        for (size_t i = 0; i < size; i++)
            n += write(buffer[i]);
        return n;
    }

    size_t print(const char *s)
    {
        if (!s)
            return 0;
        size_t n = 0;
        while (*s)
            n += write((uint8_t)*s++);
        return n;
    }

    size_t print(const String &s) { return print(s.c_str()); }
};
