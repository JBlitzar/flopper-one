#pragma once

#include "input_event.h"
#include "input_handler.h"
#include "flopper_pins.h"
#include <stack>

struct Button
{
    uint8_t pin;
    flopper::InputEvent event;
    bool was_pressed = false;
    uint32_t last_fired = 0;

    Button(uint8_t p, flopper::InputEvent e, bool wp) : pin(p), event(e), was_pressed(wp), last_fired(0) {}
    Button(uint8_t p, flopper::InputEvent e) : pin(p), event(e), was_pressed(false), last_fired(0) {}
};

Button buttons_[5] = {
    {flopper::pins::DPAD_UP, flopper::InputEvent::UP},
    {flopper::pins::DPAD_DOWN, flopper::InputEvent::DOWN},
    {flopper::pins::DPAD_CENTER, flopper::InputEvent::CENTER},
    {flopper::pins::DPAD_LEFT, flopper::InputEvent::LEFT},
    {flopper::pins::DPAD_RIGHT, flopper::InputEvent::RIGHT}};

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
            handlers.push(handler);
        }

        void pop()
        {
            if (!handlers.empty())
            {
                handlers.pop();
            }
        }

        void poll()
        {
            // poll GPIO
            // debounce
            // dispatch!
            uint32_t now = millis();
            for (auto &btn : buttons_)
            {
                bool pressed = digitalRead(btn.pin) == LOW;
                if (pressed && !btn.was_pressed && (now - btn.last_fired > 50)) // rising edge, 50 ms debounce
                {
                    dispatch_(btn.event);
                    btn.last_fired = now;
                }
                btn.was_pressed = pressed;
            }
        }

    private:
        InputDispatcher()
        {
            // init...
            for (auto &btn : buttons_)
                pinMode(btn.pin, INPUT_PULLUP);
        };
        std::stack<InputHandler *> handlers;

        void dispatch_(InputEvent event)
        {
            if (!handlers.empty())
            {
                // magic fix instead of using . notation idk
                handlers.top()->onInput(event);
            }
        }

        // copied + edited from google AI overview lol

        InputDispatcher(const InputDispatcher &) = delete;
        InputDispatcher &operator=(const InputDispatcher &) = delete;

        InputDispatcher(InputDispatcher &&) = delete;
        InputDispatcher &operator=(InputDispatcher &&) = delete;
    };
}