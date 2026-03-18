#pragma once

#include "input_event.h"
#include "input_handler.h"

#include "../desktop/cocoa_bridge.h"

#include <stack>

namespace flopper
{

    class InputDispatcher
    {
    public:
        static InputDispatcher &get_instance()
        {
            static InputDispatcher instance;
            return instance;
        }

        void push(InputHandler *handler)
        {
            handlers_.push(handler);
        }

        void pop()
        {
            if (!handlers_.empty())
                handlers_.pop();
        }

        void poll()
        {
            if (handlers_.empty())
                return;

            int raw = -1;
            while (flopper::desktop::next_input_event(raw))
            {
                InputEvent evt;
                switch (raw)
                {
                case 0:
                    evt = InputEvent::UP;
                    break;
                case 1:
                    evt = InputEvent::DOWN;
                    break;
                case 2:
                    evt = InputEvent::LEFT;
                    break;
                case 3:
                    evt = InputEvent::RIGHT;
                    break;
                default:
                    evt = InputEvent::CENTER;
                    break;
                }
                handlers_.top()->on_input(evt);
            }
        }

    private:
        std::stack<InputHandler *> handlers_;
        InputDispatcher() = default;
    };
}
