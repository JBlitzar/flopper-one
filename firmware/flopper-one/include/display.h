#pragma once

#include "flopper_pins.h"

namespace flopper
{

    class Display
    {
    public:
        static Display &get_instance()
        {
            static Display instance;
            return instance;
        }

    private:
        Display() = default;

        // copied + edited from google AI overview lol

        Display(const Display &) = delete;
        Display &operator=(const Display &) = delete;

        Display(Display &&) = delete;
        Display &operator=(Display &&) = delete;
    };
}