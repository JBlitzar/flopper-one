#pragma once

#include "input_event.h"

namespace flopper
{

    class InputHandler
    {
    public:
        virtual void onInput(InputEvent event) = 0;
        virtual ~InputHandler() = default;
    };
}