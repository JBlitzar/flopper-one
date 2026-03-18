#pragma once

#include <cstddef>
#include <cstdint>

class String;

class Print
{
public:
    virtual ~Print() = default;
    virtual size_t write(uint8_t) { return 1; }

    size_t print(const char *) { return 0; }
    size_t print(const String &) { return 0; }
};
